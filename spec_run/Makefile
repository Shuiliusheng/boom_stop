config=?
perlbench:
	./gen_ctrl.sh 400.perlbench $(config) &&\
	cd 400.perlbench &&\
	cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./perlbench_base.rv64 perlbench_base.rv64 -I./lib checkspam.pl 2500 5 25 11 150 1 1 1 1
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./perlbench_base.rv64 perlbench_base.rv64 -I./lib diffmail.pl 4 800 10 17 19 300
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./perlbench_base.rv64 perlbench_base.rv64 -I./lib splitmail.pl 1600 12 26 16 4500
gcc:
	./gen_ctrl.sh 403.gcc $(config) &&\
	cd 403.gcc &&\
	cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gcc_base.rv64 gcc_base.rv64 166.i -o 166.s
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gcc_base.rv64 gcc_base.rv64 200.i -o 200.s
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gcc_base.rv64 gcc_base.rv64 expr.i -o expr.s
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gcc_base.rv64 gcc_base.rv64 c-typeck.i -o c-typeck.s
gobmk:
	./gen_ctrl.sh 445.gobmk $(config) &&\
	cd 445.gobmk &&\
	cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gobmk_base.rv64 gobmk_base.rv64 --quiet --mode gtp < 13x13.tst
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gobmk_base.rv64 gobmk_base.rv64 --quiet --mode gtp < score2.tst
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gobmk_base.rv64 gobmk_base.rv64 --quiet --mode gtp < nngs.tst
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gobmk_base.rv64 gobmk_base.rv64 --quiet --mode gtp < trevorc.tst
	#cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./gobmk_base.rv64 gobmk_base.rv64 --quiet --mode gtp < trevord.tst
hmmer:
	./gen_ctrl.sh 456.hmmer $(config) &&\
	cd 456.hmmer &&\
	cd ./run_base_ref_rv64.0001 && perf.riscv ../../samplectrl.txt ./hmmer_base.rv64 hmmer_base.rv64 nph3.hmm swiss41

bzip2:
	./gen_ctrl.sh bzip2 $(config) &&\
	cd 401.bzip2/run_base_ref_rv64.0001i &&\
	perf.riscv ../../samplectrl.txt ./bzip2_base.rv64 bzip2_base.rv64 chicken.jpg 30

# mcf not enough memory
mcf:
	./gen_ctrl.sh mcf $(config) &&\
	cd 429.mcf/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./mcf_base.rv64 mcf_base.rv64 inp.in &&\

sjeng:
	./gen_ctrl.sh sjeng $(config) &&\
	cd 458.sjeng/run_base_ref_rv64.0001  &&\
	perf.riscv ../../samplectrl.txt ./sjeng_base.rv64 sjeng_base.rv64 ref.txt
libquantum:
	./gen_ctrl.sh libquantum $(config) &&\
	cd 462.libquantum/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./libquantum_base.rv64 libquantum_base.64 1397 8
h264ref:
	./gen_ctrl.sh h264ref $(config) &&\
	cd 464.h264ref/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./h264ref_base.rv64 h264ref_base.rv64 -d foreman_ref_encoder_baseline.cfg
omnetpp:
	./gen_ctrl.sh omnetpp $(config) &&\
	cd 471.omnetpp/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./omnetpp_base.rv64 omnetpp_base.rv64 omnetpp.ini
astar:
	./gen_ctrl.sh astar $(config) &&\
	cd 473.astar/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./astar_base.rv64 astar_base.rv64 rivers.cfg
xalancbmk:
	./gen_ctrl.sh xalancbmk $(config) &&\
	cd 483.xalancbmk/run_base_ref_rv64.0001 &&\
	perf.riscv ../../samplectrl.txt ./Xalan_base.rv64 Xalan_base.rv64 -v t5.xml xalanc.xsl

int: perlbench bzip2 gcc gobmk hmmer sjeng libquantum h264ref omnetpp astar xalancbmk
	echo “running all the int benchmarks”

subset5: sjeng libquantum gcc hmmer xalancbmk
	echo "running the five subsets"