# invoke SourceDir generated makefile for labra.pem3
labra.pem3: .libraries,labra.pem3
.libraries,labra.pem3: package/cfg/labra_pem3.xdl
	$(MAKE) -f C:\Users\allut\Documents\Koodaus\TKJ_final\JTKJ_labra_202/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\allut\Documents\Koodaus\TKJ_final\JTKJ_labra_202/src/makefile.libs clean

