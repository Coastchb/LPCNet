#./dump_data -train data/input.s16 data/features.32 data/data.u8

python src/train_lpcnet.py data/train/features.32 data/train/data.u8