all: cartoon.pdf cartoon-dt.pdf domains-fig.pdf

domains-fig.pdf: domainsfigure.py
	python3 domainsfigure.py
	pdfcrop domains-fig.pdf domains-fig.pdf

cartoon.pdf cartoon-dt.pdf: cartoon.py
	python3 cartoon.py
	pdfcrop cartoon.pdf cartoon.pdf
	pdfcrop cartoon-dt.pdf cartoon-dt.pdf

.PHONY: clean

clean:
	@rm -f *.pyc *~ cartoon.pdf cartoon-dt.pdf domains-fig.pdf

