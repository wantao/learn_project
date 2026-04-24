#!/usr/bin/env python3
"""查询大乐透前区同时含07和08的记录，按年展示"""
import os
import pymysql
from collections import defaultdict

conf = {
    'host': os.environ.get('MYSQL_HOST', '127.0.0.1'),
    'user': os.environ.get('MYSQL_USER', 'root'),
    'password': os.environ.get('MYSQL_PASSWORD', ''),
    'port': int(os.environ.get('MYSQL_PORT', 3306)),
    'db': os.environ.get('MYSQL_DB', 'lottery'),
    'charset': 'utf8mb4'
}

conn = pymysql.connect(**conf)
try:
    with conn.cursor() as cur:
        cur.execute('''
            SELECT issue, draw_date,
                   front1, front2, front3, front4, front5,
                   back1, back2
            FROM dlt_history
            WHERE 7 IN (front1, front2, front3, front4, front5)
              AND 8 IN (front1, front2, front3, front4, front5)
            ORDER BY draw_date ASC
        ''')
        rows = cur.fetchall()
finally:
    conn.close()

# 按年分组
by_year = defaultdict(list)
for row in rows:
    year = row[1].year
    by_year[year].append(row)

print(f'共查到 {len(rows)} 条记录\n')

for year in sorted(by_year):
    items = by_year[year]
    print(f'=== {year} 年（共 {len(items)} 期）===')
    print(f'{"期号":<12} {"日期":<12} {"前区":<22} {"后区":<10}')
    print('-' * 60)
    for item in items:
        issue, date, f1, f2, f3, f4, f5, b1, b2 = item
        front = f'{f1:02d} {f2:02d} {f3:02d} {f4:02d} {f5:02d}'
        back = f'{b1:02d} {b2:02d}'
        print(f'{issue:<12} {str(date):<12} {front:<22} {back:<10}')
    print()
