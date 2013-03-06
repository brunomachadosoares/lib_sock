all: 
	(cd src && $(MAKE))

clean:
	(cd src && $(MAKE) clean)
	(cd bin && rm -rf *)
