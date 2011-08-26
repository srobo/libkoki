CLEAN += docs/html docs/latex

docs:
	doxygen docs/Doxyfile

docs_latex: docs
	$(MAKE) -C docs/latex

.PHONY: docs docs_latex


