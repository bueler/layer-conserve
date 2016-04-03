obsfigures = obstacle1d.pdf ncp1d.pdf vi1d.pdf

all: $(obsfigures)

$(obsfigures): figs1d.py
	python figs1d.py

.PHONY: clean

clean:
	@rm -f *~ *1d.pdf

