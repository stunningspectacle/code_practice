#!/bin/bash

usage()
{
	echo "$0 [mrst_edv|mrst_ref|ivydale] {-r}"
	exit 1
}

project=`basename $PWD`
date=`date "+%Y%m%d%H%M"`
out=/out/${date}_${project}
build_log=out/log
build_time=out/time
target=$1
rebuild=$2

main()
{
	source ./build/envsetup.sh
	lunch $target-eng
	make $target pr1=1 -j8
#	make $target -j8
}

remove_gits()
{
	git_trees=`cat .repo/project.list`
	gits=
	for git_tree in $git_trees;do
		gits="$gits $git_tree/.git"
	done
	tar czf gits.tgz $gits;
	rm -fr $gits
}

add_gits()
{
	tar zxf gits.tgz
	echo rm gits.tgz
}

[ -z $target ] && usage
if [ x"$rebuild" == "x-r" ];then
	rm out
	cd hardware/intel/PRIVATE/pvr/
	git clean -f -d
	cd - 2>&1 > /dev/null
	mkdir $out
	ln -s $out out
fi

exec >> ${build_log}
exec 2>&1
exec 3>&2
echo CROSS_COMPILE=$CROSS_COMPILE >> ${build_time}
echo TARGET_TOOLS_PREFIX=$TARGET_TOOLS_PREFIX >> ${build_time}
echo USE_CCACHE=$USE_CCACHE >> ${build_time}
echo CCACHE_PREFIX=$CCACHE_PREFIX >> ${build_time}

date >> ${build_time}
#remove_gits
main $*
#add_gits
date >> ${build_time}
