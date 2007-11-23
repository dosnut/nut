prologues:=3;
filenametemplate "%j.mps";
input boxes;
verbatimtex
%&latex
\documentclass[8pt]{article}
\usepackage{times}
\usepackage[T1]{fontenc}
\begin{document}
\small
etex

beginfig(0);
  pair h;
  boxit.dm(btex Device Manager etex);
  h = dm.n - dm.s;

  boxjoin(h = a.n-a.s = b.n-b.s; b.w - a.e = (8mm,0));
  boxit.d1(btex Device etex);
  boxit.d2(btex Device etex);
  boxit.d3(btex \huge ... etex);
  boxit.d4();
  ypart dm.s - 3mm = ypart d1.n;
  xpart d2.c = xpart dm.c;

  boxjoin(h = a.n-a.s = b.n-b.s; b.w - a.e = (8mm,0));
  boxit.e1(btex Env. etex);
  boxit.e2(btex Env. etex);
  boxit.e3(btex \huge ... etex);
  boxit.e4();
  ypart d1.s - 3mm = ypart e1.n;
  xpart (1/2[e1.c,e2.c]) = xpart d1.c;
  xpart e3.c = xpart d2.c;

  boxjoin(h = a.n-a.s = b.n-b.s; b.w - a.e = (5mm,0));
  boxit.i1(btex Interf. etex);
  boxit.i2(btex Interf. etex);
  boxit.i3(btex \huge ... etex);
  boxit.i4("");
  ypart e1.s - 3mm = ypart i1.n;
%  xpart (1/2[i1.c,i2.c]) = xpart e1.c;
  xpart i2.c = xpart e1.c;
%  xpart i3.c + 1cm = xpart e2.c;
  
  
  drawboxed(dm, d1, d2, e1, e2, i1, i2);
  drawunboxed(d3, e3, i3);
  drawarrow dm.c .. d1.c cutbefore bpath.dm cutafter bpath.d1;
  drawarrow dm.c .. d2.c cutbefore bpath.dm cutafter bpath.d2;
  drawarrow d1.c .. e1.c cutbefore bpath.d1 cutafter bpath.e1;
  drawarrow d1.c .. e2.c cutbefore bpath.d1 cutafter bpath.e2;
  drawarrow e1.c .. i1.c cutbefore bpath.e1 cutafter bpath.i1;
  drawarrow e1.c .. i2.c cutbefore bpath.e1 cutafter bpath.i2;

  drawarrow dm.c .. d3.nw cutbefore bpath.dm;

  drawarrow d2.c .. e3.n cutbefore bpath.d2;
  drawarrow d2.c .. (e3.n+(1cm,0)) cutbefore bpath.d2;

  drawarrow e1.c .. i3.nw cutbefore bpath.e1;
  drawarrow e2.c .. (i3.n+(5mm,0)) cutbefore bpath.e2;
  drawarrow e2.c .. (i3.n+(15mm,0)) cutbefore bpath.e2;
%  drawarrow b[i].c {up} .. a[i+1].c {down}  cutafter bpath.a[i+1];
endfig;



end
