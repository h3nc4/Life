:: Compila o programa jogo-da-velha com o icone

echo 1 ICON "life.ico" > i.rc
windres i.rc -O coff -o i.res
gcc -o ..\bin\release.exe ..\src\main.c i.res
del i.res i.rc
