from io import BytesIO
from flask import Flask, send_file

bytes_per_request = 256  # should be matching with the page size of the flash
fpga_version = "s15" # allows fast switch between s15 and s50

app = Flask(__name__)


def read_slice(position: int, filename: str) -> bytes:
    with open(filename, "rb") as file:
        file.seek(position * bytes_per_request, 0)
        chunk: bytes = file.read(bytes_per_request)
    return chunk


@app.route("/getfast/<position>")
def get_file_fast(position: str):
    """
    Using positional arguments as parameter did not work!
    """
    buffer = BytesIO()
    buffer.write(
        read_slice(
            int(position), f"bin_files/{fpga_version}/blink_fast/led_test.bin"
        )
    )
    buffer.seek(0)
    return send_file(
        buffer,
        as_attachment=True,
        download_name="bitfile.bin",
        mimetype="application/octet-stream",
    )


@app.route("/getslow/<position>")
def get_file_slow(position: str):
    """
    Using positional arguments as parameter did not work!
    """
    buffer = BytesIO()
    buffer.write(
        read_slice(
            int(position), f"bin_files/{fpga_version}/blink_slow/led_test.bin"
        )
    )
    buffer.seek(0)
    return send_file(
        buffer,
        as_attachment=True,
        download_name="bitfile.bin",
        mimetype="application/octet-stream",
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)
