all: talk.pdf

cartoonfigures = cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf
cartoonfigures := $(addprefix ../cartoon/, $(cartoonfigures))

massfigures = ../refinemass/masstimeseries.pdf

$(cartoonfigures): ../cartoon/figs1d.py
	(cd ../cartoon/ && make)

$(massfigures): ../refinemass/fig.py
	(cd ../refinemass/ && make)

../anim/adv/u-1.png ../anim/sia/u-1.png: ../anim/genfigs.py
	(cd ../anim/ && make)

talk.aux: talk.tex $(massfigures) $(cartoonfigures) ../anim/adv/u-1.png ../anim/sia/u-1.png
	pdflatex talk

talk.pdf: talk.aux $(massfigures) $(cartoonfigures) ../anim/adv/u-1.png ../anim/sia/u-1.png
	pdflatex talk


.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *.nav *.snm *.toc *~

