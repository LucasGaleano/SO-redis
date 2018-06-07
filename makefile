

all: so-commons-library biblioteca parsi
	@tput setaf 2
	@echo "Terminado"
	@tput sgr0


so-commons-library:
	$(call mostrarTitulo,$@)
	git clone https://github.com/sisoputnfrba/so-commons-library ../so-commons-library
	cd ../so-commons-library; sudo make install
	@tput setaf 2
	@echo "Commons instaladas"
	@tput sgr0


parsi:
	$(call mostrarTitulo,$@)
	cd .. ; git clone https://github.com/sisoputnfrba/parsi
	cd ../parsi; sudo make install
	@tput setaf 2
	@echo "parsi  instalada"
	@tput sgr0



biblioteca:
	$(call mostrarTitulo,$@)
	cd biblioteca-propia/Debug; make all
	mkdir /usr/include/biblioteca
	cp -u ./biblioteca-propia/Debug/libbiblioteca-propia.so /usr/lib/libbiblioteca-propia.so
	cp -u ./biblioteca-propia/biblioteca/*.h /usr/include/biblioteca
	@tput setaf 2
	@echo "Biblioteca Intalada"
	@tput sgr0
	
clean:
	$(call mostrarTitulo,$@)
	rm -rf ../so-commons-library
	rm -rf /usr/include/biblioteca
	rm -rf /usr/lib/libbiblioteca-propia.so
	rm -rf ../parsi
	@tput setaf 2
	@echo "Desintalado"
	@tput sgr0

