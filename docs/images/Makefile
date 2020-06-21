.SUFFIXES:

%.svg: %.dot
	dot -T svg -o $@ $<

.PHONY: all
all: loop.svg
.PHONY: clean
clean:
	-rm -f loop.svg

