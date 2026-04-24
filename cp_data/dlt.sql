SELECT issue, draw_date,
       front1, front2, front3, front4, front5,
       back1, back2
FROM dlt_history
WHERE 7 IN (front1, front2, front3, front4, front5)
  AND 8 IN (front1, front2, front3, front4, front5)
ORDER BY draw_date ASC;