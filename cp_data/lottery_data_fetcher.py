#!/usr/bin/env python3
"""
lottery_data_fetcher.py

功能：
- 抓取双色球（ssq）历史开奖：红球6个、蓝球1个、时间、期号
- 抓取大乐透（dlt）历史开奖：前区5个、后区2个、时间、期号
- 在 MySQL（CentOS7）上创建数据库和表并写入历史数据

依赖：requests, bs4, pymysql
 pip3 install requests bs4 pymysql
 pip3 install 'beautifulsoup4<4.12

使用示例：
  export MYSQL_HOST=127.0.0.1 MYSQL_USER=root MYSQL_PASSWORD=pass
  python3 lottery_data_fetcher.py --init-db
  python3 lottery_data_fetcher.py --fetch-ssq
  python3 lottery_data_fetcher.py --fetch-dlt

"""
import os
import re
import sys
import argparse
from datetime import datetime

import requests
from bs4 import BeautifulSoup
import pymysql


def get_db_conf():
    return {
        'host': os.environ.get('MYSQL_HOST', '127.0.0.1'),
        'user': os.environ.get('MYSQL_USER', 'root'),
        'password': os.environ.get('MYSQL_PASSWORD', '密码'),
        'port': int(os.environ.get('MYSQL_PORT', 3306)),
        'db': os.environ.get('MYSQL_DB', 'lottery'),
        'charset': 'utf8mb4'
    }


def get_conn(conf, connect_db: bool = True):
    kwargs = dict(host=conf['host'], user=conf['user'], password=conf['password'], port=conf['port'], charset=conf.get('charset', 'utf8mb4'))
    if connect_db:
        kwargs['db'] = conf['db']
    return pymysql.connect(**kwargs)


def create_database_and_tables(conf):
    conn = get_conn(conf, connect_db=False)
    try:
        with conn.cursor() as cur:
            cur.execute(f"CREATE DATABASE IF NOT EXISTS `{conf['db']}` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;")
        conn.commit()
    finally:
        conn.close()

    conn = get_conn(conf, connect_db=True)
    try:
        with conn.cursor() as cur:
            cur.execute('''
            CREATE TABLE IF NOT EXISTS ssq_history (
                id INT PRIMARY KEY AUTO_INCREMENT,
                issue VARCHAR(32) NOT NULL UNIQUE,
                draw_date DATE,
                red1 TINYINT, red2 TINYINT, red3 TINYINT, red4 TINYINT, red5 TINYINT, red6 TINYINT,
                blue TINYINT
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            ''')

            cur.execute('''
            CREATE TABLE IF NOT EXISTS dlt_history (
                id INT PRIMARY KEY AUTO_INCREMENT,
                issue VARCHAR(32) NOT NULL UNIQUE,
                draw_date DATE,
                front1 TINYINT, front2 TINYINT, front3 TINYINT, front4 TINYINT, front5 TINYINT,
                back1 TINYINT, back2 TINYINT
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            ''')
        conn.commit()
    finally:
        conn.close()


def parse_date(text: str):
    for fmt in ('%Y-%m-%d', '%Y/%m/%d', '%Y.%m.%d'):
        try:
            return datetime.strptime(text.strip(), fmt).date()
        except Exception:
            continue
    # try to extract digit groups like 2020-01-01 from longer text
    m = re.search(r'(\d{4}[-/.]\d{1,2}[-/.]\d{1,2})', text)
    if m:
        return parse_date(m.group(1))
    return None


def fetch_ssq_history(conf, url=None, source_label=None):
    if url is None:
        url = 'https://datachart.500.com/ssq/history/newinc/history.php?start=00001&end=99999'
    source_label = source_label or url
    r = requests.get(url, timeout=20)
    r.raise_for_status()
    soup = BeautifulSoup(r.text, 'html.parser')
    rows = soup.select('tbody#tdata tr')
    results = []
    for row in rows:
        tds = row.find_all('td')
        if len(tds) < 16:
            continue
        issue = tds[0].get_text(strip=True)
        date = parse_date(tds[-1].get_text(strip=True))
        red = [int(tds[i].get_text(strip=True)) for i in range(1, 7)]
        blue = int(tds[7].get_text(strip=True))
        results.append({'issue': issue, 'date': date, 'red': red, 'blue': blue, 'source': source_label})
    # store
    conn = get_conn(conf)
    try:
        with conn.cursor() as cur:
            for item in results:
                try:
                    cur.execute('''INSERT INTO ssq_history (issue, draw_date, red1, red2, red3, red4, red5, red6, blue)
                                   VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)
                                ''', (
                        item['issue'], item['date'], *item['red'], item['blue']
                    ))
                except Exception:
                    # ignore row-level errors
                    continue
        conn.commit()
    finally:
        conn.close()


def fetch_dlt_history(conf, url=None, source_label=None):
    if url is None:
        url = 'https://datachart.500.com/dlt/history/newinc/history.php?start=00001&end=99999'
    source_label = source_label or url
    r = requests.get(url, timeout=20)
    r.raise_for_status()
    soup = BeautifulSoup(r.text, 'html.parser')
    rows = soup.select('tbody#tdata tr')
    results = []
    for row in rows:
        tds = row.find_all('td')
        if len(tds) < 15:
            continue
        issue = tds[0].get_text(strip=True)
        date = parse_date(tds[-1].get_text(strip=True))
        front = [int(tds[i].get_text(strip=True)) for i in range(1, 6)]
        back = [int(tds[i].get_text(strip=True)) for i in range(6, 8)]
        results.append({'issue': issue, 'date': date, 'front': front, 'back': back, 'source': source_label})
    # store
    conn = get_conn(conf)
    try:
        with conn.cursor() as cur:
            for item in results:
                try:
                    cur.execute('''INSERT INTO dlt_history (issue, draw_date, front1, front2, front3, front4, front5, back1, back2)
                                   VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s)
                                ''', (
                        item['issue'], item['date'], *item['front'], *item['back']
                    ))
                except Exception:
                    continue
        conn.commit()
    finally:
        conn.close()


def main():
    p = argparse.ArgumentParser(description='Lottery data fetcher')
    p.add_argument('--init-db', action='store_true')
    p.add_argument('--fetch-ssq', action='store_true')
    p.add_argument('--fetch-dlt', action='store_true')
    args = p.parse_args()

    conf = get_db_conf()
    if args.init_db:
        create_database_and_tables(conf)
        print('ok')
        return
    if args.fetch_ssq:
        fetch_ssq_history(conf)
        print('ok')
        return
    if args.fetch_dlt:
        fetch_dlt_history(conf)
        print('ok')
        return
    p.print_help()


if __name__ == '__main__':
    main()
