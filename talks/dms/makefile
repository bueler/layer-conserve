all: talk.pdf

cartoonfigures = cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf
cartoonfigures := $(addprefix ../cartoon/, $(cartoonfigures))

massfigures = ../refinemass/masstimeseries.pdf

obsfigures = obstacle1d.pdf ncp1d.pdf vi1d.pdf
obsfigures := $(addprefix ../obstacle/, $(obsfigures))

# no rules to make these; location = talks-public/commonfigs/; see \graphicspath{} in .tex
commonfigs = groundedscheme.pdf polaris.png supp4rignot-small.png marsh-water.jpg tsunami-sendai.jpg grnwinset.pdf

# no rules to make these; location = sia-fve/talks/; see \graphicspath{} in .tex
siafvefigs = newtonconv.pdf scale-greenland-bed-mcb.png scale-greenland-bed-old-oib.png insetinset.pdf rseps.pdf

$(cartoonfigures): ../cartoon/figs1d.py
	(cd ../cartoon/ && make)

$(massfigures): ../refinemass/fig.py
	(cd ../refinemass/ && make)

$(obsfigures): ../obstacle/figs1d.py
	(cd ../obstacle/ && make)

#../anim/adv/u-1.png ../anim/sia/u-1.png ../anim/tdep/u-1.png: ../anim/genfigs.py
#	(cd ../anim/ && make)


talk.aux: talk.tex $(cartoonfigures) $(massfigures) $(obsfigures)
	pdflatex talk

talk.pdf: talk.tex $(cartoonfigures) $(massfigures) $(obsfigures)
	pdflatex talk
	pdflatex talk


.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *.nav *.snm *.toc *~

