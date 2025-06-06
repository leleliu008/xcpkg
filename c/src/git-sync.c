#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <git2.h>

#include "core/url-transform.h"

#include "xcpkg.h"

typedef struct {
    git_indexer_progress indexerProgress;
} ProgressPayload;

// https://libgit2.org/libgit2/#HEAD/group/callback/git_transport_message_cb
static int git_transport_message_callback(const char * str, int len, void * payload) {
    (void)payload; /* unused */
	printf("remote: %.*s", len, str);
	fflush(stdout);
	return 0;
}

// https://libgit2.org/libgit2/#HEAD/group/callback/git_indexer_progress_cb
static int git_indexer_progress_callback(const git_indexer_progress * stats, void * payload) {
    ProgressPayload * progressPayload = (ProgressPayload*)payload;
    progressPayload->indexerProgress = *stats;
    return 0;
}

// https://libgit2.org/libgit2/#HEAD/group/callback/git_checkout_progress_cb
static void git_checkout_progress_callback(const char * path, size_t completed_steps, size_t total_steps, void * payload) {
    if (completed_steps == total_steps) {
        ProgressPayload * progressPayload = (ProgressPayload*)payload;
        git_indexer_progress indexerProgress = progressPayload->indexerProgress;

        if (indexerProgress.received_objects != 0) {
            printf("Receiving objects: 100%% (%u/%u), %.2f KiB, done.\n", indexerProgress.received_objects, indexerProgress.total_objects, indexerProgress.received_bytes / 1024.0);
        }

        if (indexerProgress.indexed_deltas != 0) {
            printf("Resolving deltas: 100%% (%u/%u), done.\n", indexerProgress.indexed_deltas, indexerProgress.total_deltas);
        }
    }
}

// https://libgit2.org/libgit2/#HEAD/group/credential/git_credential_ssh_key_new
// https://libgit2.org/libgit2/#HEAD/group/callback/git_credential_acquire_cb
static int git_credential_acquire_callback(git_credential ** credential, const char * url, const char * username_from_url, unsigned int allowed_types, void * payload) {
    fprintf(stderr, "git_credential_acquire_callback() url=%s\n", url);
    const char * const userHomeDIR = getenv("HOME");

    if (userHomeDIR == NULL) {
        return 1;
    }

    if (userHomeDIR[0] == '\0') {
        return 1;
    }

    size_t sshPrivateKeyFilePathLength = strlen(userHomeDIR) + 20U;
    char   sshPrivateKeyFilePath[sshPrivateKeyFilePathLength];

    int ret = snprintf(sshPrivateKeyFilePath, sshPrivateKeyFilePathLength, "%s/.ssh/id_rsa", userHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return 1;
    }

    struct stat st;

    if ((stat(sshPrivateKeyFilePath, &st) == 0) && S_ISREG(st.st_mode)) {
        git_credential_ssh_key_new(credential, username_from_url, NULL, sshPrivateKeyFilePath, NULL);
        return 0;
    }

    ret = snprintf(sshPrivateKeyFilePath, sshPrivateKeyFilePathLength, "%s/.ssh/id_ed25519", userHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return 1;
    }

    if ((stat(sshPrivateKeyFilePath, &st) == 0) && S_ISREG(st.st_mode)) {
        git_credential_ssh_key_new(credential, username_from_url, NULL, sshPrivateKeyFilePath, NULL);
        return 0;
    }

    return 1;
}

static int git_submodule_foreach_callback(git_submodule * submodule, const char * name, void * payload) {
    // git submodule update --init
    // https://libgit2.org/libgit2/#HEAD/group/submodule/git_submodule_update
    return git_submodule_update(submodule, true, NULL);
}

/**
 *  check if the given path is a empty dir
 *  error occurs, -1 is returned and errno is set to indicate the error
 *  is a empty dir, 0 is returned
 *  not a empty dir, 1 is returned
 */
static int is_empty_dir(const char * const dirpath) {
    DIR * dir = opendir(dirpath);

    if (dir == NULL) {
        return -1;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return 0;
            } else {
                int err = errno;
                closedir(dir);
                errno = err;
                return -1;
            }
        }

        const char * const p = dir_entry->d_name;

        if (p[0] == '.') {
            if (p[1] == '\0') continue;
            if (p[1] == '.') {
                if (p[2] == '\0') continue;
            }
        }

        closedir(dir);

        return 1;
    }
}

// implement following steps:
// git -c init.defaultBranch=master init
// git remote add origin https://github.com/leleliu008/xcpkg-formula-repository-official-core.git
// git fetch --progress origin +refs/heads/master:refs/remotes/origin/master
// git checkout --progress --force -B master refs/remotes/origin/master
int xcpkg_git_sync(const char * repositoryDIR, const char * remoteUrl, const char * remoteRefPath, const char * remoteTrackingRefPath, const char * checkoutToBranchName, const size_t fetchDepth) {
    //fprintf(stderr, "xcpkg_git_sync() repositoryDIR=%s remoteUrl=%s remoteRefPath=%s remoteTrackingRefPath=%s\n", repositoryDIR, remoteUrl, remoteRefPath, remoteTrackingRefPath);
    if ((repositoryDIR == NULL) || (repositoryDIR[0] == '\0')) {
        repositoryDIR = ".";
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteUrl == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteUrl[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteRefPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteRefPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (remoteTrackingRefPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (remoteTrackingRefPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (checkoutToBranchName == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (checkoutToBranchName[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    bool needInitGitRepo = false;

    struct stat st;

    if (stat(repositoryDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            switch (is_empty_dir(repositoryDIR)) {
                case -1:
                    perror(repositoryDIR);
                    return XCPKG_ERROR;
                case 0:
                    needInitGitRepo = true;
                    break;
                case 1:
                    needInitGitRepo = false;
            }
        } else {
            fprintf(stderr, "%s exist and it is not a git repository.", repositoryDIR);
            return XCPKG_ERROR;
        }
    } else {
        fprintf(stderr, "%s dir is not exist.", repositoryDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    git_libgit2_init();

    //////////////////////////////////////////////////////////////////////////////////////////////

    size_t checkoutToBranchRefPathLength = strlen(checkoutToBranchName) + 12U;
    char   checkoutToBranchRefPath[checkoutToBranchRefPathLength];

    int ret = snprintf(checkoutToBranchRefPath, checkoutToBranchRefPathLength, "refs/heads/%s", checkoutToBranchName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    git_repository   * gitRepo        = NULL;

    git_config       * gitConfig      = NULL;
    git_config_entry * gitConfigEntry = NULL;

    git_remote       * gitRemote      = NULL;

    git_reference * checkoutToBranchRefPointer = NULL;
    git_reference * remoteTrackingRefPointer = NULL;

    const git_oid * remoteTrackingRefHEADCommitOid     = NULL;
    git_object    * remoteTrackingRefHEADCommitPointer = NULL;
    git_tree      * remoteTrackingRefHEADCommitPointToTree   = NULL;

    const git_error * gitError        = NULL;

    //////////////////////////////////////////////////////////////////////////////////////////////

    char * transformedUrl = NULL;

    switch (transform_url(remoteUrl, &transformedUrl)) {
        case -1: return XCPKG_ERROR_MEMORY_ALLOCATE;
        case  1: remoteUrl = transformedUrl;
    }

    fprintf(stderr, "git Fetching: %s\n", remoteUrl);

    //////////////////////////////////////////////////////////////////////////////////////////////

    if (needInitGitRepo) {
        ret = git_repository_init(&gitRepo, repositoryDIR, false);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            free(transformedUrl);
            return abs(ret) + XCPKG_ERROR_LIBGIT2_BASE;
        }

        //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
        ret = git_remote_create(&gitRemote, gitRepo, "origin", remoteUrl);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            free(transformedUrl);
            return abs(ret) + XCPKG_ERROR_LIBGIT2_BASE;
        }
    } else {
        ret = git_repository_open_ext(&gitRepo, repositoryDIR, GIT_REPOSITORY_OPEN_NO_SEARCH, NULL);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            free(transformedUrl);
            return abs(ret) + XCPKG_ERROR_LIBGIT2_BASE;
        }

        // https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_lookup
        ret = git_remote_lookup(&gitRemote, gitRepo, "origin");

        if (ret == GIT_ENOTFOUND) {
            //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
            ret = git_remote_create(&gitRemote, gitRepo, "origin", remoteUrl);
        } else if (ret == GIT_OK) {
            ret = git_remote_set_instance_url(gitRemote, remoteUrl);
        }

        if (ret != GIT_OK) {
            gitError = git_error_last();
            fprintf(stderr, "%s\n", gitError->message);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            free(transformedUrl);
            return abs(ret) + XCPKG_ERROR_LIBGIT2_BASE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    ProgressPayload progressPayload = {0};

    git_remote_callbacks gitRemoteCallbacks = GIT_REMOTE_CALLBACKS_INIT;

	gitRemoteCallbacks.sideband_progress = git_transport_message_callback;
	gitRemoteCallbacks.transfer_progress = git_indexer_progress_callback;
	gitRemoteCallbacks.credentials       = git_credential_acquire_callback;
	gitRemoteCallbacks.payload           = &progressPayload;

    git_fetch_options gitFetchOptions = GIT_FETCH_OPTIONS_INIT;
    gitFetchOptions.callbacks = gitRemoteCallbacks;
    gitFetchOptions.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;

    // this feature was introduced in libgit2-1.7.0
#if ((LIBGIT2_VER_MAJOR == 1) && (LIBGIT2_VER_MINOR >= 7)) || (LIBGIT2_VER_MAJOR > 1)
    gitFetchOptions.depth = (int)fetchDepth;
#endif

    git_checkout_options gitCheckoutOptions = GIT_CHECKOUT_OPTIONS_INIT;
    gitCheckoutOptions.checkout_strategy    = GIT_CHECKOUT_FORCE;
    gitCheckoutOptions.progress_cb          = git_checkout_progress_callback;
    gitCheckoutOptions.progress_payload     = &progressPayload;

    if (remoteRefPath == NULL && remoteTrackingRefPath == NULL) {
        ret = git_remote_fetch(gitRemote, NULL, &gitFetchOptions, NULL);
    } else {
        size_t refspecLength = strlen(remoteRefPath) + strlen(remoteTrackingRefPath) + 2;
        char   refspec[refspecLength];

        ret = snprintf(refspec, refspecLength, "%s:%s", remoteRefPath, remoteTrackingRefPath);

        if (ret < 0) {
            perror(NULL);
            git_repository_state_cleanup(gitRepo);
            git_repository_free(gitRepo);
            git_libgit2_shutdown();
            free(transformedUrl);
            return XCPKG_ERROR;
        }

        git_strarray refspecArray = {.count = 1};
        char* strings[1] = {(char*)refspec};
        refspecArray.strings = strings;
        ret = git_remote_fetch(gitRemote, &refspecArray, &gitFetchOptions, NULL);
    }

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    git_strarray refs = {0};
    ret = git_reference_list(&refs, gitRepo);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    for (size_t i = 0U; i < refs.count; i++) {
        printf("ref:%s\n", refs.strings[i]);
        free(refs.strings[i]);
    }

    free(refs.strings);

    //////////////////////////////////////////////////////////////////////////////////////////////

    ret = git_reference_lookup(&remoteTrackingRefPointer, gitRepo, remoteTrackingRefPath);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    remoteTrackingRefHEADCommitOid = git_reference_target(remoteTrackingRefPointer);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    ret = git_branch_lookup(&checkoutToBranchRefPointer, gitRepo, checkoutToBranchName, GIT_BRANCH_LOCAL);

    if (ret == GIT_OK) {
        const git_oid * checkoutToBranchHEADOid = git_reference_target(checkoutToBranchRefPointer);

        if (NULL != checkoutToBranchHEADOid) {
            // remote tracking branch's SHA-1 is equal to locale tracking branch's HEAD SHA-1, means no need to perform merge
            if (memcmp(remoteTrackingRefHEADCommitOid->id, checkoutToBranchHEADOid->id, 20) == 0) {
                ret = git_repository_set_head(gitRepo, checkoutToBranchRefPath);

                if (ret != GIT_OK) {
                    gitError = git_error_last();
                }

                goto finalize;
            }
        }
    }

    if (ret == GIT_ENOTFOUND) {
        ret = git_reference_create(&checkoutToBranchRefPointer, gitRepo, checkoutToBranchRefPath, remoteTrackingRefHEADCommitOid, false, NULL);
    }

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    // git cat-file -p FETCH_HEAD
    // tree 29bd7db599c429dd821a8564196a02daebe09465
    // parent eb86afc806d4df01763663c1f2a9b411bddf3a82
    // author leleliu008 <leleliu008@gmail.com> 1673391639 +0800
    // committer leleliu008 <leleliu008@gmail.com> 1673391639 +0800
    //
    // optimized

    // https://libgit2.org/libgit2/#HEAD/group/reference/git_reference_peel
    ret = git_reference_peel(&remoteTrackingRefHEADCommitPointer, remoteTrackingRefPointer, GIT_OBJ_COMMIT);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    // https://libgit2.org/libgit2/#HEAD/group/commit/git_commit_tree
    ret = git_commit_tree(&remoteTrackingRefHEADCommitPointToTree, (git_commit*)remoteTrackingRefHEADCommitPointer);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    // https://libgit2.org/libgit2/#HEAD/group/checkout/git_checkout_tree
    ret = git_checkout_tree(gitRepo, (git_object*)remoteTrackingRefHEADCommitPointToTree, &gitCheckoutOptions);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    {
        git_reference * checkoutToBranchRefPointerNew = NULL;

        // https://libgit2.org/libgit2/#HEAD/group/reference/git_reference_set_target
        ret = git_reference_set_target(&checkoutToBranchRefPointerNew, checkoutToBranchRefPointer, remoteTrackingRefHEADCommitOid, NULL);

        git_reference_free(checkoutToBranchRefPointerNew);

        if (ret != GIT_OK) {
            gitError = git_error_last();
            goto finalize;
        }
    }

    ret = git_repository_set_head(gitRepo, checkoutToBranchRefPath);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    // https://libgit2.org/libgit2/#HEAD/group/submodule/git_submodule_foreach
    ret = git_submodule_foreach(gitRepo, git_submodule_foreach_callback, gitRepo);

    if (ret != GIT_OK) {
        gitError = git_error_last();
    }

finalize:
    if (ret == GIT_OK) {
        printf("%s\n", "Already up to date.");
    } else {
        if (gitError != NULL) {
            fprintf(stderr, "%s\n", gitError->message);
        }
    }

    if (transformedUrl != NULL) {
        free(transformedUrl);
    }

    git_repository_state_cleanup(gitRepo);

    git_repository_free(gitRepo);
    git_remote_free(gitRemote);

    git_reference_free(remoteTrackingRefPointer);
    git_object_free(remoteTrackingRefHEADCommitPointer);
    git_tree_free(remoteTrackingRefHEADCommitPointToTree);

    git_reference_free(checkoutToBranchRefPointer);

    git_config_free(gitConfig);
    git_config_entry_free(gitConfigEntry);

    git_libgit2_shutdown();

    return ret == GIT_OK ? XCPKG_OK : abs(ret) + XCPKG_ERROR_LIBGIT2_BASE;
}
