rm -rf doc_tmp/
mkdir doc_tmp
cd software/z80_emulator/doc/
./build.sh
cd - > /dev/null
cp -R software/z80_emulator/doc/*.pdf doc_tmp/
unoconv -f pdf aufbau_und_funktionsweise_eines_prozessors_facharbeit.odt
mv aufbau_und_funktionsweise_eines_prozessors_facharbeit.pdf doc_tmp/
pdfunite doc_tmp/*.pdf facharbeit_prozessoren_komplett.pdf
