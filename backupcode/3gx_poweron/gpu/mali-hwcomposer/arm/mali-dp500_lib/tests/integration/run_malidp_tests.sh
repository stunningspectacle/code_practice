mkdir -p /data/data/com.mali.testjava

if [ "$1" == 1 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_ion_buffer_simple_post" --testid=1 -v
elif [ "$1" == 2 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_iommu_page_fault" --testid=2 -v
elif [ "$1" == 3 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_rotation_non_compressed" --testid=3 -v
elif [ "$1" == 4 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_rotation_afbc_compressed" --testid=4 -v
elif [ "$1" == 5 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_source_crop_yuv_incremental" --testid=5 -v
elif [ "$1" == 6 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_source_crop_rgb_afbc" --testid=6 -v
elif [ "$1" == 7 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_performance_complex_scenes" --testid=7 -v
elif [ "$1" == 8 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_scaling_composition_sweep_one_plane" --testid=8 -v
elif [ "$1" == 9 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_scaling_composition_sweep_two_plane" --testid=9 -v
elif [ "$1" == 10 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_scaling_composition_sweep_three_plane" --testid=10 -v
elif [ "$1" == 11 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_scaling_composition_three_layers" --testid=11 -v
elif [ "$1" == 12 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_acquire_fence" --testid=12 -v
elif [ "$1" == 13 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_complete_fence" --testid=13 -v
elif [ "$1" == 14 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_memory_write_complete_fence" --testid=14 -v
elif [ "$1" == 15 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_memory_writeout_composition" --testid=15 -v
elif [ "$1" == 16 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_multi_layer_worst_case" --testid=16 -v
elif [ "$1" == 17 ]; then
	malidp_integration_tests --suite="kernel_integration" --test="test_driver_resource_sanity" --testid=17 -v
elif [ "$1" == 18 ]; then
	malidp_integration_tests --suite="core_integration" --test="multi_pass_through" --testid=0 -v
elif [ "$1" == 19 ]; then
	malidp_integration_tests --suite="core_integration" --test="yv12_multi_pass" --testid=1 -v
else
	echo "!!!Not a valid testid!!!"
fi

