.SUFFIXES:

%.svg: %.dot
	dot -T svg -o $@ $<

IMAGES = loop.svg ex1.svg

.PHONY: all
all: $(IMAGES)
.PHONY: clean
clean:
	-rm -f $(IMAGES)


