CLEAN += bugs/html

bugs_html: bugs/html/index.html

bugs/html/index.html: bugs/*.yaml
	ditz html bugs/html
