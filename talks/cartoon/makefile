cartoonfigures = cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf

all: $(cartoonfigures)

$(cartoonfigures): figs1d.py
	python figs1d.py
	pdfcrop cartoon-sets.pdf cartoon-sets.pdf

.PHONY: clean

clean:
	@rm -f *~ *cartoon*.pdf

