version = $(shell bin/ipkg --version | head -n 1 | awk '{print $$2}')
filename= ipkg-$(version).tar.gz

dist: bin/ipkg zsh-completion/_ipkg
	@tar zvcf $(filename) $^ && \
	command -v openssl > /dev/null && \
    openssl sha256 $(filename) && exit 0; \
    command -v sha256sum > /dev/null && \
    ha256sum $(filename)

copy:
	cp $$(brew --prefix ipkg)/bin/ipkg bin/
	cp $$(brew --prefix ipkg)/share/zsh/site-functions/_ipkg zsh-completion/_ipkg

clean:
	rm $(filename)

.PHONY: clean copy
