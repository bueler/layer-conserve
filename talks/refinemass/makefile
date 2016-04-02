all: masstimeseries.pdf

# slowish; requires ../../petsc/layer compiled from ../../petsc/layer.c
mass0.txt mass1.txt mass2.txt:
	./generate.sh

masstimeseries.pdf: fig.py mass0.txt
	python fig.py -o masstimeseries.pdf mass?.txt

.PHONY: clean

clean:
	@rm -f *~
