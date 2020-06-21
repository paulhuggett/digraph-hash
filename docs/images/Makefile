.SUFFIXES:

%.svg: %.dot
	dot -T svg -o $@ $<

IMAGES = \
	loop.svg \
	ex1.svg \
	ex2.svg \
	self_loop.svg

.PHONY: all
all: $(IMAGES)
.PHONY: clean
clean:
	-rm -f $(IMAGES)


