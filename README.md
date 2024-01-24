# branch_prediction
Branch predictor simulator to evaluate different configurations of branch predictors

This project was a part of course ECE 563 (Microprocessor Architecture) at NC State University under Dr. Eric Rotenburg.

This project implemented 3 configurations of branch predictors:
1. Bimodal Branch Predictor
2. Gshare Branch Predictor
3. Hybrid Branch Predictor (was a combination of Bimodal and Gshare predictor models)


Instructions to run project:
1. Type "make" to build.  (Type "make clean" first if you already compiled and want to recompile from scratch.)

2. Run trace reader:

   To run without throttling output:
   ./sim bimodal 6 gcc_trace.txt
   ./sim gshare 9 3 gcc_trace.txt
   ./sim hybrid 8 14 10 5 gcc_trace.txt

   To run with throttling (via "less"):
   ./sim bimodal 6 gcc_trace.txt | less
   ./sim gshare 9 3 gcc_trace.txt | less
   ./sim hybrid 8 14 10 5 gcc_trace.txt | less

The report provides context to the results obtained by the different predictor configurations.
