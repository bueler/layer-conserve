all: lc.pdf

figures = cartoon.pdf cartoon-dt.pdf domains-fig.pdf

domains-fig.pdf: domainsfigure.py
	python domainsfigure.py
	pdfcrop domains-fig.pdf domains-fig.pdf

cartoon.pdf cartoon-dt.pdf: cartoon.py
	python cartoon.py
	pdfcrop cartoon.pdf cartoon.pdf
	pdfcrop cartoon-dt.pdf cartoon-dt.pdf

lc.pdf: lc.aux lc.bbl lc.tex $(figures)
	pdflatex lc
	pdflatex lc

lc.aux: lc.tex $(figures)
	pdflatex lc
	bibtex lc

lc.bbl: lc.aux lc.bib
	bibtex lc

.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *~ $(figures)

