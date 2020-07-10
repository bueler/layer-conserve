all: lc.pdf

lcpublish.zip: lc.pdf lc.bbl
	rm -rf lcpublish/
	mkdir lcpublish/
	mkdir lcpublish/figs/
	cp -f figs/*.pdf lcpublish/figs/
	mkdir lcpublish/tikz/
	cp -f tikz/*.tex lcpublish/tikz/
	cp -f siamart190516.cls siamplain.bst lc.bib lc.bbl lc.tex lcpublish/
	zip -r lcpublish lcpublish

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
	rm -f *.out *.aux *.log *.bbl *.blg *.thm *.synctex.gz *~
	(cd figs/ && make clean)
	rm -rf lcpublish/ lcpublish.zip

