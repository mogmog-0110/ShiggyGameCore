/// @file spatial_query.cpp
/// @brief Quadtree・Gridの空間検索を実演するサンプル

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "sgc/spatial/Quadtree.hpp"
#include "sgc/spatial/Grid.hpp"

/// @brief 敵データ
struct Enemy
{
	std::string name;
	float x;
	float y;
};

int main()
{
	std::cout << "--- Spatial Query Demo ---" << std::endl;
	std::cout << std::endl;

	// ── テストデータ（敵の配置） ────────────────────────────
	const std::vector<Enemy> enemies = {
		{"Goblin_A",   50.0f,  50.0f},
		{"Goblin_B",   80.0f,  30.0f},
		{"Skeleton_A", 200.0f, 150.0f},
		{"Skeleton_B", 220.0f, 180.0f},
		{"Dragon",     500.0f, 500.0f},
		{"Slime_A",    10.0f,  10.0f},
		{"Slime_B",    900.0f, 900.0f},
		{"Wolf_A",     300.0f, 400.0f},
		{"Wolf_B",     350.0f, 420.0f},
		{"Boss",       750.0f, 750.0f},
	};

	// ═══════════════════════════════════════════════════════
	// Part 1: Quadtree
	// ═══════════════════════════════════════════════════════
	std::cout << "========== Quadtree ==========" << std::endl;
	std::cout << std::endl;

	/// ワールド全体を覆う四分木を作成（1000x1000）
	sgc::Quadtreef qt(sgc::AABB2f{{0.0f, 0.0f}, {1000.0f, 1000.0f}});

	/// 全敵を挿入（各敵は10x10のバウンディングボックスを持つ）
	std::vector<sgc::Quadtreef::Handle> handles;
	for (const auto& enemy : enemies)
	{
		constexpr float halfSize = 5.0f;
		auto h = qt.insert(sgc::AABB2f{
			{enemy.x - halfSize, enemy.y - halfSize},
			{enemy.x + halfSize, enemy.y + halfSize}
		});
		handles.push_back(h);
	}
	std::cout << "Inserted " << qt.size() << " enemies into Quadtree" << std::endl;
	std::cout << std::endl;

	// ── 範囲クエリ: プレイヤー周辺の敵を検索 ────────────────
	std::cout << "=== Range Query: area (0,0)-(100,100) ===" << std::endl;
	const auto rangeResults = qt.queryRange(sgc::AABB2f{{0.0f, 0.0f}, {100.0f, 100.0f}});
	std::cout << "  Found " << rangeResults.size() << " enemies:" << std::endl;
	for (const auto h : rangeResults)
	{
		const auto& bounds = qt.getBounds(h);
		const auto center = bounds.center();
		std::cout << "    " << enemies[h].name
			<< " at (" << center.x << ", " << center.y << ")" << std::endl;
	}
	std::cout << std::endl;

	// ── 半径クエリ: 円形範囲検索 ────────────────────────────
	std::cout << "=== Radius Query: center=(300,400), radius=100 ===" << std::endl;
	const auto radiusResults = qt.queryRadius({300.0f, 400.0f}, 100.0f);
	std::cout << "  Found " << radiusResults.size() << " enemies:" << std::endl;
	for (const auto h : radiusResults)
	{
		const auto& bounds = qt.getBounds(h);
		const auto center = bounds.center();
		std::cout << "    " << enemies[h].name
			<< " at (" << center.x << ", " << center.y << ")" << std::endl;
	}
	std::cout << std::endl;

	// ── k近傍探索 ───────────────────────────────────────────
	std::cout << "=== K-Nearest: point=(200,200), k=3 ===" << std::endl;
	const auto knnResults = qt.findKNearest({200.0f, 200.0f}, 3);
	std::cout << "  Nearest 3 enemies:" << std::endl;
	for (const auto h : knnResults)
	{
		const auto& bounds = qt.getBounds(h);
		const auto center = bounds.center();
		std::cout << "    " << enemies[h].name
			<< " at (" << center.x << ", " << center.y << ")" << std::endl;
	}
	std::cout << std::endl;

	// ── レイキャスト ────────────────────────────────────────
	std::cout << "=== Raycast: origin=(0,0), direction=(1,1) ===" << std::endl;
	const auto rayHits = qt.raycast(sgc::Ray2f{{0.0f, 0.0f}, {1.0f, 1.0f}});
	std::cout << "  Hit " << rayHits.size() << " enemies:" << std::endl;
	for (const auto& hit : rayHits)
	{
		const auto& bounds = qt.getBounds(hit.handle);
		const auto center = bounds.center();
		std::cout << "    " << enemies[hit.handle].name
			<< " at (" << center.x << ", " << center.y << ")"
			<< " distance=" << hit.distance << std::endl;
	}
	std::cout << std::endl;

	// ═══════════════════════════════════════════════════════
	// Part 2: Grid（タイルベース空間管理）
	// ═══════════════════════════════════════════════════════
	std::cout << "========== Grid ==========" << std::endl;
	std::cout << std::endl;

	/// 20x20のグリッドを作成（各セルは50x50ワールド単位）
	constexpr int GRID_W = 20;
	constexpr int GRID_H = 20;
	constexpr float CELL_SIZE = 50.0f;
	sgc::Grid<int> grid(GRID_W, GRID_H, 0);

	/// 敵をグリッドセルに配置（セルの値は敵の数を表す）
	std::cout << "=== Placing Enemies on Grid ===" << std::endl;
	for (const auto& enemy : enemies)
	{
		auto coord = sgc::Grid<int>::worldToGrid(sgc::Vec2f{enemy.x, enemy.y}, CELL_SIZE);
		if (grid.isValid(coord.x, coord.y))
		{
			grid.at(coord) += 1;
			std::cout << "  " << enemy.name
				<< " -> cell(" << coord.x << ", " << coord.y << ")" << std::endl;
		}
	}
	std::cout << std::endl;

	// ── 隣接セルの探索（4方向） ─────────────────────────────
	/// プレイヤーがセル(1,1)にいる場合の周囲を確認
	const sgc::GridCoord playerCell{1, 1};
	std::cout << "=== Neighbors of cell(1,1) - 4 directions ===" << std::endl;
	grid.forEachNeighbor4(playerCell, [](int x, int y, int& cell)
	{
		std::cout << "  cell(" << x << ", " << y << "): "
			<< cell << " enemies" << std::endl;
	});
	std::cout << std::endl;

	// ── グリッド座標⇔ワールド座標変換 ──────────────────────
	std::cout << "=== Coordinate Conversion ===" << std::endl;
	const sgc::GridCoord testCoord{4, 3};
	const auto worldPos = sgc::Grid<int>::gridToWorld(testCoord, CELL_SIZE);
	std::cout << "  grid(4,3) -> world(" << worldPos.x << ", " << worldPos.y << ")" << std::endl;

	const auto backToGrid = sgc::Grid<int>::worldToGrid(worldPos, CELL_SIZE);
	std::cout << "  world(" << worldPos.x << ", " << worldPos.y
		<< ") -> grid(" << backToGrid.x << ", " << backToGrid.y << ")" << std::endl;
	std::cout << std::endl;

	// ── ブレゼンハムの直線 ──────────────────────────────────
	std::cout << "=== Bresenham Line: (0,0) to (5,3) ===" << std::endl;
	const auto lineCoords = sgc::Grid<int>::line({0, 0}, {5, 3});
	std::cout << "  Path: ";
	for (const auto& c : lineCoords)
	{
		std::cout << "(" << c.x << "," << c.y << ") ";
	}
	std::cout << std::endl;
	std::cout << std::endl;

	// ── A*パスファインディング ──────────────────────────────
	std::cout << "=== A* Pathfinding ===" << std::endl;

	/// 壁を配置して迷路を作成
	sgc::Grid<int> maze(10, 10, 0);
	/// 1 = 壁
	for (int y = 1; y <= 7; ++y)
	{
		maze.at(3, y) = 1;
	}
	for (int x = 5; x <= 8; ++x)
	{
		maze.at(x, 4) = 1;
	}

	/// 迷路の表示
	std::cout << "  Maze (0=open, 1=wall):" << std::endl;
	for (int y = 0; y < maze.height(); ++y)
	{
		std::cout << "    ";
		for (int x = 0; x < maze.width(); ++x)
		{
			std::cout << maze.at(x, y) << " ";
		}
		std::cout << std::endl;
	}

	/// A*探索
	const sgc::GridCoord start{0, 0};
	const sgc::GridCoord goal{9, 9};

	auto path = sgc::findPathAStar<int>(
		maze, start, goal,
		[](sgc::GridCoord, sgc::GridCoord, const int& cell) -> float
		{
			/// 壁なら通行不可（負のコスト）
			if (cell == 1) return -1.0f;
			return 1.0f;
		},
		sgc::heuristic::manhattan
	);

	if (path.empty())
	{
		std::cout << "  No path found!" << std::endl;
	}
	else
	{
		std::cout << "  Path found (" << path.size() << " steps):" << std::endl;
		std::cout << "    ";
		for (const auto& c : path)
		{
			std::cout << "(" << c.x << "," << c.y << ") ";
		}
		std::cout << std::endl;

		/// パスを迷路に描画
		std::cout << "  Maze with path (2=path):" << std::endl;
		for (const auto& c : path)
		{
			if (maze.at(c) == 0)
			{
				maze.at(c) = 2;
			}
		}
		for (int y = 0; y < maze.height(); ++y)
		{
			std::cout << "    ";
			for (int x = 0; x < maze.width(); ++x)
			{
				const int v = maze.at(x, y);
				if (v == 2) std::cout << "* ";
				else if (v == 1) std::cout << "# ";
				else std::cout << ". ";
			}
			std::cout << std::endl;
		}
	}

	return 0;
}
