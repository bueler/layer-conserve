all: talk.pdf

figures = cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf obstacle1d.pdf ncp1d.pdf vi1d.pdf newtonconv.pdf scale-greenland-bed-mcb.png scale-greenland-bed-old-oib.png insetinset.pdf rseps.pdf
#figures := $(addprefix chaps/, $(addsuffix .tex, $(chapsroots)))

SIAFVETALKS = ../../../sia-fve/talks/

cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf: $(SIAFVETALKS)figs1d.py
	python $(SIAFVETALKS)figs1d.py
	pdfcrop cartoon-sets.pdf cartoon-sets.pdf

obstacle1d.pdf ncp1d.pdf vi1d.pdf: $(SIAFVETALKS)obstacle1d.py
	python $(SIAFVETALKS)obstacle1d.py

newtonconv.pdf: $(SIAFVETALKS)newtonconv.pdf
	ln -s $(SIAFVETALKS)newtonconv.pdf

insetinset.pdf: $(SIAFVETALKS)insetinset.pdf
	ln -s $(SIAFVETALKS)insetinset.pdf

rseps.pdf: $(SIAFVETALKS)rseps.pdf
	ln -s $(SIAFVETALKS)rseps.pdf

scale-greenland-bed-mcb.png: $(SIAFVETALKS)scale-greenland-bed-mcb.png
	ln -s $(SIAFVETALKS)scale-greenland-bed-mcb.png

scale-greenland-bed-old-oib.png: $(SIAFVETALKS)scale-greenland-bed-old-oib.png
	ln -s $(SIAFVETALKS)scale-greenland-bed-old-oib.png

#talk.aux: talk.tex $(figures)
#	pdflatex talk

talk.pdf: talk.tex $(figures)
	pdflatex talk
	pdflatex talk


.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *.nav *.snm *.toc *~
	@rm -f cartoon*.pdf *1d.pdf
	@rm -f newtonconv.pdf scale-greenland-bed*png insetinset.pdf rseps.pdf
