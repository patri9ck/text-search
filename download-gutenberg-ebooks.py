#!/usr/bin/env python3

import os
import re

import requests
from bs4 import BeautifulSoup

BASE_URL = "https://www.gutenberg.org"
TOP_100_URL = "https://www.gutenberg.org/browse/scores/top"
OUTPUT_DIR = "data"

HEADERS = {
    "User-Agent": "Mozilla/5.0 (Gutenberg downloader; respectful use)"
}


def get_top_100_ebook_ids():
    print("Fetching Top 100 list...")

    response = requests.get(TOP_100_URL, headers=HEADERS)

    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")

    header = soup.find("h2", string=re.compile("Top 100 EBooks last 30 days"))

    if not header:
        raise RuntimeError("Could not find Top 100 section.")

    ol = header.find_next_sibling("ol")
    ebook_ids = []

    for li in ol.find_all("li"):
        link = li.find("a")

        if link and "ebooks" in link["href"]:
            ebook_id = link["href"].split("/")[-1]

            if ebook_id.isdigit():
                ebook_ids.append(ebook_id)

    return ebook_ids


def download_plain_text(ebook_id):
    txt_urls = [
        f"{BASE_URL}/files/{ebook_id}/{ebook_id}-0.txt",
        f"{BASE_URL}/files/{ebook_id}/{ebook_id}.txt"
    ]

    for url in txt_urls:
        r = requests.get(url, headers=HEADERS)

        if r.status_code == 200:
            return r.text

    return None


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    ebook_ids = get_top_100_ebook_ids()

    print(f"Found {len(ebook_ids)} ebooks.")

    for idx, ebook_id in enumerate(ebook_ids, start=1):
        print(f"[{idx}/100] Downloading ebook {ebook_id}...")
        text = download_plain_text(ebook_id)

        if text:
            file_path = os.path.join(OUTPUT_DIR, f"{ebook_id}.txt")

            with open(file_path, "w", encoding="utf-8") as f:
                f.write(text)
        else:
            print(f"No plain text found for ebook {ebook_id}")

    print("Done!")


if __name__ == "__main__":
    main()
