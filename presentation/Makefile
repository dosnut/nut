
.PHONY:all
all: presentation.pdf handout.pdf

SRC=${wildcard *.tex}
SRC+=${wildcard *.sty}
SRC+=${wildcard *.png}
SRC+=${wildcard *.jpg}
SRC+=nutsstructure.pdf
SRC+=libnutstructure.pdf

PRESTMP=presentation.nav presentation.out presentation.snm presentation.vrb
PRESTMP+=presentation.aux  presentation.log presentation.toc

HANDTMP=handout.nav handout.out handout.snm handout.vrb
HANDTMP+=handout.aux  handout.log handout.toc

presentation.pdf: ${SRC}
	@echo Removing temporary files
	@rm -f ${PRESTMP}
	@pdflatex presentation.tex
	@pdflatex presentation.tex
	@echo Removing temporary files
	@rm -f ${PRESTMP}

handout.pdf: ${SRC}
	@echo Removing temporary files
	@rm -f ${HANDTMP}
	@pdflatex handout.tex
	@pdflatex handout.tex
	@echo Removing temporary files
	@rm -f ${HANDTMP}

nutsstructure.pdf: nutsstructure.mp
	@echo [MPOST] nutsstructure.mp
	@mpost nutsstructure.mp
	@echo "[EPS->PDF]" nutsstructure.mps
	@epstopdf nutsstructure.mps
	@echo Removing temporary files
	@rm -f nutsstructure.mps nutsstructure.log nutsstructure.mpx

libnutstructure.pdf: libnutstructure.mp
	@echo [MPOST] libnutstructure.mp
	@mpost libnutstructure.mp
	@echo "[EPS->PDF]" libnutstructure.mps
	@epstopdf libnutstructure.mps
	@echo Removing temporary files
	@rm -f libnutstructure.mps libnutstructure.log libnutstructure.mpx

.PHONY: clean
clean:
	@echo Removing presentation.pdf handout.pdf
	@rm -f presentation.pdf handout.pdf
