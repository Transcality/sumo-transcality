# S3 Output for SUMO

SUMO can write any of its outputs directly to Amazon S3 (or an S3-compatible store such as MinIO)
by giving an `s3://bucket/key` URL instead of a file name. Combined with SUMO's native
Parquet output this puts simulation results straight into a data lake.

---

## How it works

- **Parquet** output comes from upstream SUMO (`ParquetFormatter`, see the
  [SUMO documentation on output formats](https://sumo.dlr.de/docs/Simulation/Output/index.html)).
  An output is written as Parquet if its file name ends with `.parquet` or if
  `--output.format parquet` is given. CSV (`.csv`) and XML work the same way.
- **S3** is provided by `OutputDevice_S3` (`src/utils/iodevices/OutputDevice_S3.*`). Every output
  whose name starts with `s3://` is streamed to S3 with Apache Arrow's S3 filesystem
  (multipart upload), independent of the format. The object becomes visible once the output is
  closed, i.e. at the end of the simulation.

Since the S3 device is just another output device, all outputs and options of SUMO work with it
(`--fcd-output`, `--tripinfo-output`, `edgeData`/`laneData` files, `--output-prefix`, ...).

---

## Building

S3 support needs Apache Arrow and Parquet built with S3 support (e.g. Homebrew's `apache-arrow`
or the Apache Arrow APT packages). It is enabled by default whenever Parquet is enabled and Arrow
provides S3:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_PARQUET=ON -DENABLE_S3=ON
cmake --build build -j8
```

The CMake output lists `Parquet` and `S3` under "Enabled features". Arrow >= 24 requires C++20
(the default on macOS; use `-DCMAKE_CXX_STANDARD=20` on Linux), otherwise SUMO disables Parquet
and S3. The `Dockerfile` builds an image with both enabled.

---

## Configuration

Without further configuration the default AWS credential chain (`AWS_ACCESS_KEY_ID`,
`~/.aws/credentials`, instance roles, ...) and region are used. The following environment
variables override them:

| Variable                | Description                                                        |
|-------------------------|--------------------------------------------------------------------|
| `SUMO_S3_ACCESS_KEY`    | Access key ID (used together with `SUMO_S3_SECRET_KEY`)            |
| `SUMO_S3_SECRET_KEY`    | Secret access key                                                  |
| `SUMO_S3_SESSION_TOKEN` | Session token for temporary credentials (optional)                 |
| `SUMO_S3_REGION`        | Region of the bucket                                               |
| `SUMO_S3_ENDPOINT`      | Endpoint of an S3-compatible store, e.g. `http://localhost:9000`   |

Parquet specific settings are SUMO options:

| Option                   | Description                                                        |
|--------------------------|--------------------------------------------------------------------|
| `--output.format`        | Format if not derivable from the file name (`xml`, `csv`, `parquet`) |
| `--output.compression`   | Compression of Parquet output (e.g. `zstd`, `snappy`, `gzip`)      |
| `--output.column-header` | How column names are derived (`none`, `tag`, `auto`, `plain`, `full`) |

---

## Usage examples

```bash
export SUMO_S3_REGION=eu-central-1
sumo -c scenario.sumocfg \
     --fcd-output s3://sumo-traffic-bucket/run42/fcd.parquet \
     --tripinfo-output s3://sumo-traffic-bucket/run42/tripinfo.parquet
```

```xml
<additional>
    <edgeData id="edge_data" file="s3://sumo-traffic-bucket/run42/edge_data.parquet" period="300"/>
    <laneData id="lane_data" file="s3://sumo-traffic-bucket/run42/lane_data.parquet" period="300"/>
</additional>
```

Local test with MinIO:

```bash
export SUMO_S3_ENDPOINT=http://localhost:9000 SUMO_S3_ACCESS_KEY=minioadmin SUMO_S3_SECRET_KEY=minioadmin
sumo -c scenario.sumocfg --fcd-output s3://test/fcd.parquet
```

---

## Notes

- Compressed output with `.gz` is not supported for S3; use Parquet compression instead.
- Errors when connecting or opening the object are reported at startup, errors when completing
  the upload at the end of the simulation.
- Earlier versions of this fork had their own Parquet writers (`OutputDevice_Parquet`,
  `OutputDevice_ParquetUnstructured`) and the variables `SUMO_PARQUET_COMPRESSION`,
  `SUMO_PARQUET_ROWGROUP_SIZE` and `SUMO_PARQUET_BUFFER_SIZE`. They were replaced by upstream's
  Parquet output, which also derives the columns of outputs with varying attributes
  (e.g. `edgeData`) automatically.
