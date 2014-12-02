#!/usr/bin/python

import sys
import ROOT
from array import array

nCycle = int(sys.argv[1])
outputFile = sys.argv[2]

pars_all = [[[0. for k in range(nCycle)] for j in range(24)] for i in range(3)]
for i in range(nCycle):

	results_all = [results.strip().split() for results in open('align_mille_'+str(i+1)+'.txt', 'r').readlines()]
	for j in range(24):
		for k in range(3):
			pars_all[k][j][i] = float(results_all[j][k])
			print k, j, i, pars_all[k][j][i]

## make trend plots
canvas = ROOT.TCanvas()
canvas.Print(outputFile + '[')

id_iter = array('f', range(1, nCycle+1))
for iPar in range(3):
	for iDetector in range(24):

		pars_iter = array('f', pars_all[iPar][iDetector])
		gr = ROOT.TGraph(nCycle, id_iter, pars_iter)
		gr.SetTitle('Detector: %d, Parameter: %d' % (iDetector+1, iPar+1))

		canvas.cd()
		gr.Draw('AC*')
		canvas.Print(outputFile) 

canvas.Print(outputFile + ']')