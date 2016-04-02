all: talk.pdf

cartoonfigures = cartoon-wclimate.pdf cartoon-sensitive-one.pdf cartoon-sensitive-two.pdf cartoon-sensitive-three.pdf cartoon-sets.pdf
cartoonfigures := $(addprefix ../cartoon/, $(cartoonfigures))

massfigures = ../refinemass/masstimeseries.pdf

$(cartoonfigures): ../cartoon/figs1d.py
	(cd ../cartoon/ && make)

$(massfigures): ../refinemass/fig.py
	(cd ../refinemass/ && make)

../../petsc/layer: ../../petsc/layer.c
	(cd ../../petsc/ && make layer)

anim/adv/u-1.png: ../genfigs.py ../../petsc/layer
	(cd anim/ && rm -rf adv/ && mkdir adv/)
	../../petsc/layer -lay_noshow -lay_genfigs anim/adv/ -da_refine 3 -lay_dt 0.04 -lay_steps 50 -lay_adscheme 1 -lay_jac
	python ../genfigs.py anim/adv/ --stride 1 --uscale 12.0

anim/sia/u-1.png: ../genfigs.py ../../petsc/layer
	(cd anim/ && rm -rf sia/ && mkdir sia/)
	../../petsc/layer -lay_noshow -lay_genfigs anim/sia/ -da_refine 3 -lay_dt 0.02 -lay_steps 1000 -lay_adscheme 1 -lay_lambda 1.0 -lay_gamma 0.1 -lay_glenn 3.0 -lay_jac
	python ../genfigs.py anim/sia/ --stride 20 --uscale 0.9


talk.aux: talk.tex $(massfigures) $(cartoonfigures) anim/adv/u-1.png anim/sia/u-1.png
	pdflatex talk

talk.pdf: talk.aux $(massfigures) $(cartoonfigures) anim/adv/u-1.png anim/sia/u-1.png
	pdflatex talk


.PHONY: clean

clean:
	@rm -f *.pyc *.out *.aux *.log *.bbl *.blg *.synctex.gz *.nav *.snm *.toc *~
	(cd anim/ && rm -rf adv/ sia/)

