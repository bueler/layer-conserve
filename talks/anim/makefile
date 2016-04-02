all: adv/u-1.png sia/u-1.png tdep/u-1.png

../../petsc/layer: ../../petsc/layer.c
	(cd ../../petsc/ && make layer)

adv/u-1.png: genfigs.py ../../petsc/layer
	rm -rf adv/
	mkdir adv/
	../../petsc/layer -lay_noshow -lay_genfigs adv/ -da_refine 3 -lay_dt 0.04 -lay_steps 50 -lay_adscheme 1 -lay_jac
	python genfigs.py adv/ --stride 1 --uscale 12.0

sia/u-1.png: genfigs.py ../../petsc/layer
	rm -rf sia/
	mkdir sia/
	../../petsc/layer -lay_noshow -lay_genfigs sia/ -da_refine 3 -lay_dt 0.02 -lay_steps 1000 -lay_adscheme 1 -lay_lambda 1.0 -lay_gamma 0.1 -lay_glenn 3.0 -lay_jac
	python genfigs.py sia/ --stride 20 --uscale 0.9

tdep/u-1.png: genfigs.py ../../petsc/layer
	rm -rf tdep/
	mkdir tdep/
	../../petsc/layer -lay_noshow -lay_genfigs tdep/ -da_refine 3 -lay_dt 0.04 -lay_steps 250 -lay_adscheme 1 -lay_jac -lay_timedependentsource
	python genfigs.py tdep/ --stride 1 --uscale 12.0 --taxis 250


.PHONY: clean

clean:
	@rm -f *~
	@rm -rf adv/ sia/ tdep/

