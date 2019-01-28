Number System Converter / Konvertor číselných soustav
=====================================================

[![Build Status](https://travis-ci.org/ondrg/nsc.png?branch=master)](https://travis-ci.org/ondrg/nsc)

Základní informace
------------------

Aplikace slouží k převodu čísel mezi číselnými soustavami o základu 2 až 36.

Použití
-------

Pro čtení se používá standardní vstup, pro výpis standardní výstup.

**Formát vstupu:** `[XXX]Z1=Z2`  
`XXX` = číslo ve vstupní soustavě  
`Z1` = vstupní soustava  
`Z2` = výstupní soustava (do které má být číslo převedeno)  
Např.: `[1011]2=10`

**Formát výstupu:** `[XXX]Z2`  
`XXX` = číslo ve výstupní soustavě  
`Z2` = výstupní soustava  
Např.: `[11]10`

Spuštění v Dockeru
------------------

```bash
$ docker build --tag=nsc .
$ echo '[42]10=2' | docker run -i --rm nsc
```

Reprezentace čísel
------------------

Pro reprezentaci číselných hodnot větších než 9 se používají písmena A až Z.
Použití malých písmen (a až z) není povoleno.
