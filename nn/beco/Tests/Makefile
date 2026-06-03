BECO_TEST_ITEM ?= BECO_NN_TEST

# BECO_DSP_TEST
# BECO_NN_TEST

ifeq ($(BECO_TEST_ITEM), BECO_DSP_TEST)
obj-y += beco_dsp/
ccflags-y += -DBECO_DSP_TEST
endif

ifeq ($(BECO_TEST_ITEM), BECO_NN_TEST)
obj-y += beco_nn/
ccflags-y += -DBECO_NN_TEST
endif

obj-y += beco_test.o
subdir-ccflags-y += -Inn/beco/Tests \
                    -Itests/hwtest \
                    -Iutils/hwtimer_list
