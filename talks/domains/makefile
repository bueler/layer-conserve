all: domainfig.pdf

domainfig.pdf: fig.py
	python fig.py
	pdfcrop domainfig.pdf domainfig.pdf

.PHONY: clean

clean:
	@rm -f *~ *.pyc *.pdf

