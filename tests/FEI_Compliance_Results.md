# REDHAWK FEI Compliance Test Results

## Execution

To run [test\_FmRdsSimulator\_FEI.py](test_FmRdsSimulator_FEI.py), execute the following command from the `FmRdsSimulator/tests` directory:

```
python test_FmRdsSimulator_FEI.py
```

The test may take several minutes to perform the checks when successful. It is common that fewer checks are made when unexpected failures occur, which prevent all checks from being made.

## Results

### Summary Report

```
Report Statistics:
   Checks that returned "FAIL" .................. 1
   Checks that returned "info" .................. 1
   Checks that returned "no" .................... 16
   Checks that returned "ok" .................... 235
   Checks with silent results ................... 70
   Total checks made ............................ 323
```

* `FAIL` indicates the test failed. It may be acceptable to fail a test depending on the device/design. See below.
* `WARN` CAN be fine, and may just be informational. See below.
* `info` is fine, just informational for developer to confirm the intended results. See below.
* `no` is fine, just informational for developer to confirm the intended results. Indicates an optional field was not used.
* `ok` is good, and indicates a check passed.
* `silent results` are checks that passed but do not appear anywhere in the output unless they fail.

### `info` Details

The `info` check is reporting that it is impossible to test full multi-out port capability with a single channel. The features that cannot be tested are not necessary for a single channel anyway.

```
testFRONTEND_3_4_DataFlow1: dataFloat_out: multi-out test: Cannot fully
     test multiport because only single RX_DIGITIZER tuner capability.......info
```

### `FAIL` Details

There is 1 check that report `FAIL` with the FmRdsSimulator, and this is known.

#### Multi-out port checks

The FmRdsSimulator does not implement a multi-out port, which is acceptable since the FmRdsSimulator is a single channel device, but is considered bad practice nonetheless. It was done intentionally in order to make the FmRdsSimulator device as easy to use for a beginner as possible, without knowing all necessary steps to make use of a device with a multi-out port. This of course means the multi-out tests will fail. These failures are all expected.

```
testFRONTEND_3_4_DataFlow1: dataFloat_out: multi-out test: Stream not
     received on port with bad connection ID................................FAIL
```

### `ERROR` Details

In addition to creating a human readable summary, the compliance test raises an `AssertionError` when tests fail to allow automated testing and test reporting. Due to the `FAIL` mentioned above, the following associated `ERROR` is reported.

```
ERROR: RX_DIG 4 DataFlow - First Port
```
