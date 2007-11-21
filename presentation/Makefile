
.PHONY:all
all: presentation.pdf

SRC=${wildcard *.tex}
SRC+=${wildcard *.sty}
SRC+=${wildcard *.png}
SRC+=${wildcard *.jpg}
SRC+=nuts_structure.pdf

TMP=presentation.nav presentation.out presentation.snm presentation.vrb
TMP+=presentation.aux  presentation.log presentation.toc

presentation.pdf: ${SRC}
	@echo Removing temporary files
	@rm -f ${TMP}
	@pdflatex presentation.tex
	@pdflatex presentation.tex
	@echo Removing temporary files
	@rm -f ${TMP}

.PHONY: clean
clean:
	@echo Removing presentation.pdf
	@rm -f presentation.pdf
