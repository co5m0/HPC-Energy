# `results/` folder

Contains one folder for each [`src/`](../src) folders.

- [`matrix_mul/`](./matrix_mul) results from matrix multiplication execution
- [`reduce/`](./reduce) results from dot product execution

## Files format

In these folders there are two different type of file:

- Energy measurements: `test_with_power.csv`
- Power measurements: `<type of optimization>_<input size>_<number of threads>_power_<timestamp>.csv`

### **Energy results** in [`test_with_power.csv`](./matrix_mul/test_with_power.csv) (for _matrix multiplication_) or [`test_with_power.csv`](./reduce/test_with_power.csv) (for _dot product_) for each colum have:

1. **Namefile**, type of optimization

- [SEQ](../src/matrix_mul/matrix_mul_seq_v2.c)
- [MULTI](../src/matrix_mul/matrix_mul_multi_v2.c)
- [SIMD](../src/matrix_mul/matrix_mul_simd_v2.c)
- [DOT-PRODUCT MULTI](../src/reduce/reduce_multi.c)
- [DOT-PRODUCT SIMD](../src/reduce/reduce_simd_v2.c)

2. **Time**, execution time in _sec_

3. **Size**, one side matrix size (since the matrix is square the total number of elements are _Size _ Size\*)

4. **Energy**, total energy consumption measured during the computation

5. **Nthread**, number of threads used

### Every test execution produce a **power measurements** file with every domain available on the columns.

## Test eviroment

All test were executed on a server test with these specs:

**CPU** _Intel Xeon E5-2430 v2_  
**RAM** _32 GB @ 1600 MHz_  
**OS** _Ubuntu 20.04.2 LTS 4.15.0-144-generic_
