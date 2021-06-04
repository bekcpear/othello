EMACS=emacs

DEPFILES = publish muse-init.el common.css

%.html: %.muse $(DEPFILES)
	@echo Generating XHTML for $<...
	EMACS=$(EMACS) SITEFLAG=$(SITEFLAG) $(abs_srcdir)/publish othello-xhtml $<

%.pdf: %.muse $(DEPFILES)
	@echo Generating PDF for $<...
	EMACS=$(EMACS) SITEFLAG=$(SITEFLAG) $(abs_srcdir)/publish othello-pdf $<

MUSE_DOCS = \
	InstallationGuide.muse \
	AIPluginWritingTutorial.muse

MUSE_DOCS_HTML = $(patsubst %.muse, %.html, $(MUSE_DOCS))

#pdf support is disabled
#MUSE_DOCS_PDF = $(patsubst %.muse, %.pdf, $(MUSE_DOCS))

docs : $(MUSE_DOCS_HTML) $(MUSE_DOCS_PDF)

EXTRA_DIST = \
	muse-init.el	\
	publish \
	common.css \
	$(MUSE_DOCS)

CLEANFILES = $(MUSE_DOCS_HTML) $(MUSE_DOCS_PDF)

