all: talk.pdf

# no rules to make these; location = talks-public/commonfigs/; see \graphicspath{} in .tex
commonfigs = groundedscheme.pdf polaris.png supp4rignot-small.png marsh-water.jpg tsunami-sendai.jpg grnwinset.pdf

# no rules to make these; location = sia-fve/talks/; see \graphicspath{} in .tex
siafvefigs = newtonconv.pdf scale-greenland-bed-mcb.png scale-greenland-bed-old-oib.png insetinset.pdf rseps.pdf

# rules to make these:
figures = cartoon-wclimate.pdf obstacle1d.pdf ncp1d.pdf vi1d.pdf
#figures := $(addprefix chaps/, $(addsuffix .tex, $(chapsroots)))

SIAFVETALKS = ../../../sia-fve/talks/

cartoon-wclimate.pdf: $(SIAFVETALKS)figs1d.py
	python $(SIAFVETALKS)figs1d.py

obstacle1d.pdf ncp1d.pdf vi1d.pdf: $(SIAFVETALKS)obstacle1d.py
	python $(SIAFVETALKS)obstacle1d.py

talk.aux: talk.tex $(figures)
	pdflatex talk

talk.pdf: talk.tex $(figures)
	pdflatex talk
	pdflatex talk


.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *.nav *.snm *.toc *~
	@rm -f cartoon*.pdf *1d.pdf

