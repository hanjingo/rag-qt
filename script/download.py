import hashlib
import os
import sys
import time
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

MODELS = {
    "qwen2.5-0.5b-instruct-q4_0.gguf": {
        "url": "https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF/resolve/main/qwen2.5-0.5b-instruct-q4_0.gguf",
        "sha256": "7671c0c304e6ce5a7fc577bcb12aba01e2c155cc2efd29b2213c95b18edaf6ed",
    },
    "ggml-silero-v6.2.0.bin": {
        "url": "https://huggingface.co/ggml-org/whisper-vad/resolve/main/ggml-silero-v6.2.0.bin",
        "sha256": "2aa269b785eeb53a82983a20501ddf7c1d9c48e33ab63a41391ac6c9f7fb6987",
    },
    "ggml-base.bin": {
        "url": "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin",
        "sha256": "60ed5bc3dd14eea856493d334349b405782ddcaf0028d4b5df4088345fba2efe",
    },
    "bge-small-en-v1.5-q8_0.gguf": {
        "url": "https://huggingface.co/ggml-org/bge-small-en-v1.5-Q8_0-GGUF/resolve/main/bge-small-en-v1.5-q8_0.gguf",
        "sha256": "f046db1dc724cf4f6f0a0c5917e922823b73eb1d27b8f9a9c2797f7866974804",
    },
}

def verify_sha256(file_path: Path, expected_sha256: str) -> bool:
    """Calculate the SHA256 hash in chunks to avoid using too much memory for large files."""
    if not file_path.exists():
        return False
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4 * 1024 * 1024), b""):
            hasher.update(chunk)
    computed = hasher.hexdigest().lower()
    return computed == expected_sha256.lower()

def download_file_with_retry(
    url: str,
    dest_path: Path,
    expected_sha256: str,
    max_retries: int = 3,
    retry_delay: int = 5
) -> bool:
    """Download a file with retries and SHA256 verification."""
    # 1. Check whether the local file already exists and is valid.
    if dest_path.exists():
        print(f"[CHECK] Verifying existing local file: {dest_path.name} ...")
        if verify_sha256(dest_path, expected_sha256):
            print(f"[SKIP] SHA256 verification passed; skipping download: {dest_path.name}\n")
            return True
        else:
            print(f"[WARN] Local file SHA256 mismatch; deleting and downloading again: {dest_path.name}")
            dest_path.unlink()

    req = Request(
        url,
        headers={"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
    )

    # 2. Download with retries.
    for attempt in range(1, max_retries + 1):
        print(f"[START] Downloading {dest_path.name} (attempt {attempt}/{max_retries})...")
        try:
            with urlopen(req, timeout=30) as response, open(dest_path, "wb") as out_file:
                total_size = int(response.headers.get("Content-Length", 0))
                downloaded = 0
                chunk_size = 2 * 1024 * 1024  # 2 MB buffer
                last_log_time = time.time()

                while True:
                    chunk = response.read(chunk_size)
                    if not chunk:
                        break
                    out_file.write(chunk)
                    downloaded += len(chunk)

                    current_time = time.time()
                    if total_size > 0 and (current_time - last_log_time > 2.0 or downloaded == total_size):
                        percent = (downloaded / total_size) * 100
                        mb_downloaded = downloaded / (1024 * 1024)
                        mb_total = total_size / (1024 * 1024)
                        print(f" -> {dest_path.name}: {percent:.1f}% ({mb_downloaded:.1f}/{mb_total:.1f} MB)")
                        last_log_time = current_time

            # 3. Verify the SHA256 checksum immediately after downloading.
            print(f"[VERIFY] Checking file integrity with SHA256...")
            if verify_sha256(dest_path, expected_sha256):
                print(f"[SUCCESS] {dest_path.name} downloaded and verified successfully!\n")
                return True
            else:
                print(f"[ERROR] Attempt {attempt} failed: SHA256 checksum mismatch!")
                if dest_path.exists():
                    dest_path.unlink()

        except (HTTPError, URLError, Exception) as e:
            print(f"[ERROR] An exception occurred during attempt {attempt}: {e}")
            if dest_path.exists():
                dest_path.unlink()

        if attempt < max_retries:
            print(f"[RETRY] Retrying in {retry_delay} seconds...\n")
            time.sleep(retry_delay)

    print(f"[FATAL] Failed to download or verify the file after {max_retries} attempts: {dest_path.name}")
    return False

def main():
    if len(sys.argv) > 1:
        models_dir = Path(sys.argv[1])
    else:
        models_dir = Path(__file__).parent / "models"

    models_dir.mkdir(parents=True, exist_ok=True)
    print(f"=== Model output directory: {models_dir.resolve()} ===\n")

    all_success = True
    for filename, info in MODELS.items():
        dest = models_dir / filename
        success = download_file_with_retry(
            url=info["url"],
            dest_path=dest,
            expected_sha256=info["sha256"],
            max_retries=3,
            retry_delay=5
        )
        if not success:
            all_success = False

    if not all_success:
        print("=== Some model files failed to download or verify ===")
        sys.exit(1)

    print("=== All model files downloaded successfully and passed SHA256 verification ===")

if __name__ == "__main__":
    main()