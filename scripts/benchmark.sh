#!/usr/bin/env bash

set -e

# Profile one version against another / multiple other versions, for overall time / RAM, and for component breakdown using Rprof

script_dir=`dirname "$0"`
cd "${script_dir}"/../
STITCH_HOME=`pwd`
export PATH=`pwd`/:${PATH}
mkdir -p benchmark-results

#1.2.5 1.2.9 1.3.2 1.3.4
for version in 1.3.6 1.3.5
do
    cd ${STITCH_HOME}
    echo start $version
    # I think below should work on other machines
    # Basically want the user location for libraries
    r_libs=`R --slave -e ".libPaths()[1]" | awk '{print substr($0, 5, 1000)}' | tr -d '"'`
    rm -r -f ${r_libs}/00LOCK-STITCH
    echo ${STITCH_HOME}/releases/STITCH_${version}.tar.gz
    export SEQLIB_ROOT=${STITCH_HOME}/SeqLib/
    R CMD INSTALL ${STITCH_HOME}/releases/STITCH_${version}.tar.gz
    cd ${STITCH_HOME}/test-data/mouse_data/
    #  CRAMS BAMS
    for use in BAMS
    do
	export USE=${use}
	export OUTPUTDIR=${STITCH_HOME}/test-results/benchmark_${use}_${version}/
	export N_CORES=1
	export TITLE=${use}_${version}
	export OUTPUT_PLOT=${STITCH_HOME}/benchmark-results/benchmark_${use}_${version}.pdf
	/usr/bin/time -v ${STITCH_HOME}/scripts/profile.R 2>&1 | tee ${STITCH_HOME}/benchmark-results/time_${use}_${version}.txt
    done
done

exit 0
