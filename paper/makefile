all: lc.pdf

figs/cartoon.pdf figs/cartoon-dt.pdf figs/domains-fig.pdf:
	(cd figs/ && make)

lc.pdf: lc.aux lc.bbl lc.tex
	pdflatex lc
	pdflatex lc

lc.aux: lc.tex figs/cartoon.pdf figs/cartoon-dt.pdf figs/domains-fig.pdf
	pdflatex lc
	bibtex lc

lc.bbl: lc.aux lc.bib
	bibtex lc

.PHONY: clean

clean:
	@rm -f *.out *.aux *.log *.bbl *.blg *.thm *.synctex.gz *~
	(cd figs/ && make clean)

