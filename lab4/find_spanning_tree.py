#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "folium",
#     "geopy",
#     "networkx",
#     "osmnx",
# ]
# ///
import subprocess
import csv
import os
import sys
from pathlib import Path
import pandas as pd

import folium

# простите мне глобалы?👉🏻👈🏻
INPUT_FILE = "data/buildings.csv"
OUTPUT_FILE = "data/spanning_tree.csv"
C_EXECUTABLE = "./spanning_tree"



def run_c_processor(input_path: str, output_path: str) -> bool:
    """
    Запускает скомпилированный C-файл.
    Передаёт пути к входному и выходному файлам как аргументы.
    """

    if not os.path.isfile(C_EXECUTABLE):
        print(f"Не найден исполняемый файл: {C_EXECUTABLE}")
        print(f"Скомпилируйте: gcc path_processor.c -o {C_EXECUTABLE} -lm")
        return False
    
    try:
        result = subprocess.run(
            [C_EXECUTABLE, ],
            capture_output=True,
            text=True,
            check=True,
            timeout=30
        )
    except subprocess.CalledProcessError as e:
        print(f"Ошибка выполнения C: {e}")
        if e.stderr:
            print(f"stderr: {e.stderr}")
        return False
    except Exception as e:
        print(f"Ошибка запуска: {e}")
        return False
    
    if result.stdout:
        print(f"C-вывод: {result.stdout.strip()}")
    
    return True


def draw_map(df_in: pd.DataFrame, df_out: pd.DataFrame, output_html: str = "route_map.html"):
    """
    Отрисовывает интерактивную карту с маршрутом через Folium.
    Сохраняет результат в HTML-файл.
    """

    if df_out.empty:
        print("Пустое остовное дерево, нечего рисовать")
        return
    
    center = df_in['lat'].mean(), df_in['lon'].mean()
    m = folium.Map(location=center, zoom_start=13, tiles='OpenStreetMap')

    mapping: dict[int, dict[str, float]] = df_in.set_index('id').to_dict(orient='index')
    
    for i, row in df_out.iterrows():
        node_id_from, node_id_to = int(row['node_id_from']), int(row['node_id_to'])
        
        lat1, lon1 = mapping[node_id_from]['lat'], mapping[node_id_from]['lon']
        lat2, lon2 = mapping[node_id_to]['lat'], mapping[node_id_to]['lon']

        folium.PolyLine(
            locations=[(lat1, lon1), (lat2, lon2)],
            color='blue',
            weight=4,
            opacity=0.8,
            tooltip=f"Ребро {i+1}: {node_id_from} → {node_id_to}"
        ).add_to(m)
    
    m.save(output_html)
    print(f"Карта сохранена: {output_html}")
    print(f"Откройте файл в браузере для просмотра")


def main():
    print("Поиск остовного дерева")
    print("=" * 50)
    
    print(f"\nЗапуск C-обработчика...")
    if not run_c_processor(INPUT_FILE, OUTPUT_FILE):
        print("Ошибка выполнения C-кода")
        sys.exit(1)
    
    if not os.path.isfile(OUTPUT_FILE):
        print(f"Не найден выходной файл: {OUTPUT_FILE}")
        sys.exit(1)
    
    df_in = pd.read_csv(INPUT_FILE, sep='\t')
    df_out = pd.read_csv(OUTPUT_FILE, sep='\t')
    print(f"Прочитано остовное дерево из {len(df_out)} домов")
    
    print(f"\nОтрисовка карты...")
    draw_map(df_in, df_out)
    
    print("\nГотово!")


if __name__ == "__main__":
    main()