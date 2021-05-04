
# for maint branch:
#include ${PETSC_DIR}/conf/variables
#include ${PETSC_DIR}/conf/rules

# for master branch:
include ${PETSC_DIR}/lib/petsc/conf/variables
include ${PETSC_DIR}/lib/petsc/conf/rules

layer: layer.o  chkopts
	-${CLINKER} -o layer layer.o  ${PETSC_SNES_LIB}
	${RM} layer.o

doclayer.pdf: doclayer.tex
	pdflatex doclayer.tex
	pdflatex doclayer.tex

.PHONY: clean

clean::
	@rm -f *~ layer *.txt
	@rm -f *~ *.aux *.log *.synctex.g
