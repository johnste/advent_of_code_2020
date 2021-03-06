//
//  main.cpp
//  AdventOfCode201220_1
//
//  Created by Kjell-Olov Högdal on 2020-12-20.
//

#include <iostream>
#include <vector>
#include <numeric>
#include <functional>
#include <map>
#include <array>
#include <sstream>

extern char const* pSingleTile;
extern char const* pExample;
extern char const* pData;

const unsigned int TILE_SIDE_SIZE = 10;
auto const TILE_COLUMN_COUNT = TILE_SIDE_SIZE;
auto const TILE_ROW_COUNT = TILE_SIDE_SIZE;

using Dummy = int;
using Pixel = char;
using Vector = std::array<Pixel, TILE_COLUMN_COUNT>;
using Vectors = std::vector<Vector>;
using Tile = std::array<Vector, TILE_ROW_COUNT>; // [row][column] Tile
using TileID = unsigned int;
using Tiles = std::map<TileID,Tile>;
using TileIDs = std::vector<TileID>;
using EdgeHash = std::size_t;
using TileHashes = std::vector<EdgeHash>;
using TilesHashes = std::map<TileID,TileHashes>;
using AdjacentMap = std::map<EdgeHash,TileIDs>;

using Result = std::uint64_t; // 10 to the power of 4+4+4+4 = 16  OK! 50 bits ok :)

void print_vector(Vector const& v) {
    for (auto const& c : v) {
        std::cout << c;
    }
}

void print_tile(Tile const& tile, TileID tile_id) {
    std::cout << "\n<tile >" << tile_id;
    for (auto const& v : tile) {
        std::cout << "\n\t";
        print_vector(v);
    }
}

void print_tile_edge_hashes(TileHashes const& hashes, TileID tile_id) {
    std::cout << "\n" << ":";
    for (auto const& h : hashes) std::cout << " " << h;
}

void print_adjacent_map(AdjacentMap const&  edge_hash_map) {
    std::cout << "\nprint_adjacent_map";
    for (auto const& adjacent_map_entry : edge_hash_map) {
        std::cout << "\n\thash:" << adjacent_map_entry.first;
        for (auto const& id : adjacent_map_entry.second) {
            std::cout << " id:" << id;
        }
    }
}

Vector pixel_vector_from_string(std::string const& sVector) {
    Vector result {};
    for (int i=0;i<sVector.size();i++) {
        result[i] = sVector[i];
    }
    return result;
}

bool get_tile(std::istream& in,Tile& tile,TileID& tile_id) {
    std::string sEntry {};
    std::vector<std::string> sTileEntries {};
    while (std::getline(in, sEntry)) {
        // split input on empty line
        if (sEntry.size()>0) sTileEntries.push_back(sEntry);
        else break;
    }
    if (sEntry.size()>0) sTileEntries.push_back(sEntry); // No empty line after last entry
    for (int i = 0;i<sTileEntries.size();i++) {
        if (i==0) {
            std::string sIndex {sTileEntries[i].begin()+5,sTileEntries[i].end()};
            tile_id = std::stoi(sIndex);
        }
        else {
            tile[i-1] = pixel_vector_from_string(sTileEntries[i]);
        }
    }
    return (sTileEntries.size()>0);
}

Tiles tiles_from_string_literal(char const* pData) {
    Tiles result {};
    std::basic_istringstream<char> in {pData};
    Tile tile {};
    TileID tile_id {};
    while (get_tile(in, tile,tile_id)) {
        result[tile_id] = tile;
    }
    return result;
}

enum e_RotationDirection {
    eClockwise
    ,eAnticlockwise
};

template <class C>
void print_container(C const& c) {
    for (auto const& m : c) std::cout << " " << m;
}

EdgeHash edge_hash(Vector const& v) {
    EdgeHash result;
    std::string sVector {};
    for (int i=0;i<v.size();i++) {
        sVector += v[i];
    }
    result = std::hash<std::string>{}(sVector);
    return result;
}

Vectors border_vectors(Tile const& tile,e_RotationDirection rot_dir) {
    Vectors result {4, {TILE_SIDE_SIZE}}; // clockwise top,right,bottom,left. anticlockwise top,left,bottom,right
    for (int index = 0; index < TILE_SIDE_SIZE; index++) {
        switch (rot_dir) {
            case eClockwise: {
                result[0][index] = tile[0][index]; // top
                result[1][index] = tile[index].back(); // right
                result[2][index] = tile.back()[TILE_SIDE_SIZE-index-1];// bottom
                result[3][index] = tile[TILE_SIDE_SIZE-index-1][0];// left
            } break;
            case eAnticlockwise: {
                result[0][index] = tile[0][TILE_SIDE_SIZE-index-1];//top
                result[1][index] = tile[index][0];// left
                result[2][index] = tile.back()[index];//bottom
                result[3][index] = tile[TILE_SIDE_SIZE-index-1].back();//right
            } break;
        }
    }
    return result;
}

TileHashes vector_hashes(Vectors const& vs) {
    TileHashes result;
    for (auto const& v : vs) {
        result.push_back(edge_hash(v));
    }
    return result;
}

TileHashes tile_edge_hashes(Tile const& tile,e_RotationDirection rot_dir) {
    TileHashes result {vector_hashes(border_vectors(tile, rot_dir))};
    return result;
}

TilesHashes tiles_edge_hashes(Tiles const& tiles,e_RotationDirection rot_dir) {
    TilesHashes result {};
    for (auto const& tile_entry : tiles) {
        result[tile_entry.first] = tile_edge_hashes(tile_entry.second,rot_dir);
        // print_tile_edge_hashes(result[tile_entry.first],tile_entry.first);
    }
    return result;
}

// Map border hash to Tile Id
AdjacentMap adjacent_map(TilesHashes const& tile_id_edge_hashes) {
    AdjacentMap result;
    for (auto const& tiles_hash_entry : tile_id_edge_hashes) {
        for (auto const& hash : tiles_hash_entry.second) {
            result[hash].push_back(tiles_hash_entry.first); // map hash to tile ID
        }
    }
    return result;
}

AdjacentMap merge_maps(AdjacentMap const& clockwise_edge_hash_map,AdjacentMap const& anticlockwise_edge_hash_map) {
    AdjacentMap result;
    for (auto const& entry : clockwise_edge_hash_map) {
        for (auto const& id : entry.second) {
            result[entry.first].push_back(id);
        }
    }
    for (auto const& entry : anticlockwise_edge_hash_map) {
        for (auto const& id : entry.second) {
            result[entry.first].push_back(id);
        }
    }
    return result;
}

TilesHashes connection_map(AdjacentMap const& am) {
    TilesHashes result;
    for (auto const& entry : am) {
        for (auto const& id : entry.second) {
            result[id].push_back(entry.first);
        }
    }
    return result;
}

int main(int argc, const char * argv[]) {
    // Hm... This may be better understodd as a "cover" problem jigsaw-style?
    // Prepare for an adjacent matrix approach?
    // a) We need a way to uniquely identify edges between tiles
    // Idea: Use a hash of the four edge vectors of each tile!
    
    // Ok. We now have a table of Tile IDs and their four edges hash values.
    // If we now turn this table into one keyed on Hash value we will group tiles that is adjacent (same hash)
    // AHA -
    // Two tiles that share a Hash (edge) is connected :)
    
    // AHA -
    // We need to account for rotated and flipped tiles!
    // We can do this by creating two versions of "border" keys (hashes)
    // a) clockwise border keys (+ keys)
    // b) anti clockwise border keys (- keys)
    //
    // When a "+ key" is equal to a  "- key" we know these tiles potentially
    // are adjecent to eachother (flipped or rotated).
    // In a graph model we can use Tile IDs as vertices and "border keys" as edges.
    
    if (false) {
        // Single tile test
        std::basic_istringstream<char> in {pSingleTile};
        Tile tile {};
        TileID tile_id {};
        get_tile(in, tile, tile_id);
        print_tile(tile, tile_id);
        {
            auto clockwise_borders = border_vectors(tile,eClockwise);
            std::cout << "\nclockwise borders";
            for (auto const& v : clockwise_borders) {
                std::cout << "\n\tv:";
                print_vector(v);
            }
        }
        {
            auto anticlockwise_borders = border_vectors(tile,eAnticlockwise);
            std::cout << "\nAnticlockwise borders";
            for (auto const& v : anticlockwise_borders) {
                std::cout << "\n\tv:";
                print_vector(v);
            }
            auto vh = vector_hashes(anticlockwise_borders);
            std::cout << "\nhashes:";
            print_container(vh);
        }
    }
    if (false) {
        // Test
        Tiles tiles {tiles_from_string_literal(pExample)};
        auto clockwise_tile_id_edge_hashes = tiles_edge_hashes(tiles,eClockwise);
        auto anticlockwise_tile_id_edge_hashes = tiles_edge_hashes(tiles,eAnticlockwise);

        auto clockwise_edge_hash_map = adjacent_map(clockwise_tile_id_edge_hashes);
        print_adjacent_map(clockwise_edge_hash_map);
        auto anticlockwise_edge_hash_map = adjacent_map(anticlockwise_tile_id_edge_hashes);
        print_adjacent_map(anticlockwise_edge_hash_map);
        
        auto merged = merge_maps(clockwise_edge_hash_map, anticlockwise_edge_hash_map);
        // Every entry with two IDs are now matched :) (remove the others)
        AdjacentMap mapped {};
        std::copy_if(merged.begin(), merged.end(), std::inserter(mapped, mapped.begin()), [](auto const& entry) {
            return entry.second.size() > 1;
        });
        print_adjacent_map(mapped);
        // Now count how many connected borders each tile has
        auto connections = connection_map(mapped);
        for (auto const& entry : connections) {
            std::cout << "\nid:" << entry.first << " connect count:" << entry.second.size();
            for (auto const& h : entry.second) {
                std::cout << " h:" << h;
            }
        }
        // Each tile connecttion counted twice (bidirectional graph)
        TileIDs corners;
        for (auto const& entry : connections) {
            if (entry.second.size()==4) corners.push_back(entry.first);
        }
                                                        
        Result result_init {1};
        auto result = std::accumulate(corners.begin(), corners.end(), result_init,std::multiplies<Result>{});
        
        std::cout << "\n\nPart 1 example answer = " << result;
    }
    
    if (true) {
        Tiles tiles {tiles_from_string_literal(pData)};
        auto clockwise_tile_id_edge_hashes = tiles_edge_hashes(tiles,eClockwise);
        auto anticlockwise_tile_id_edge_hashes = tiles_edge_hashes(tiles,eAnticlockwise);

        auto clockwise_edge_hash_map = adjacent_map(clockwise_tile_id_edge_hashes);
        print_adjacent_map(clockwise_edge_hash_map);
        auto anticlockwise_edge_hash_map = adjacent_map(anticlockwise_tile_id_edge_hashes);
        print_adjacent_map(anticlockwise_edge_hash_map);
        
        auto merged = merge_maps(clockwise_edge_hash_map, anticlockwise_edge_hash_map);
        // Every entry with two IDs are now a matched "edge" :) (remove the others)
        AdjacentMap mapped {};
        std::copy_if(merged.begin(), merged.end(), std::inserter(mapped, mapped.begin()), [](auto const& entry) {
            return entry.second.size() > 1;
        });
        print_adjacent_map(mapped);
        // Now count how many connected borders each tile has
        auto connections = connection_map(mapped);
        for (auto const& entry : connections) {
            std::cout << "\nid:" << entry.first << " connect count:" << entry.second.size();
            for (auto const& h : entry.second) {
                std::cout << " h:" << h;
            }
        }
        // Each tile connecttion counted twice (bidirectional graph)
        // Corner tiles has 2 unidirectional connections (count 4 in our map)
        TileIDs corners;
        for (auto const& entry : connections) {
            if (entry.second.size()==4) corners.push_back(entry.first);
        }
                                                        
        Result result_init {1};
        auto result = std::accumulate(corners.begin(), corners.end(), result_init,std::multiplies<Result>{});
        
        std::cout << "\n\nPart 1 answer = " << result;

    }
    
    std::cout << "\n\n";
    return 0;
}

char const* pSingleTile = R"(Tile 2311:
..##.#..#.
##..#.....
#...##..#.
####.#...#
##.##.###.
##...#.###
.#.#.#..##
..#....#..
###...#.#.
..###..###)";

char const* pExampleResult = R"(1951    2311    3079
2729    1427    2473
2971    1489    1171)";
char const* pExample = R"(Tile 2311:
..##.#..#.
##..#.....
#...##..#.
####.#...#
##.##.###.
##...#.###
.#.#.#..##
..#....#..
###...#.#.
..###..###

Tile 1951:
#.##...##.
#.####...#
.....#..##
#...######
.##.#....#
.###.#####
###.##.##.
.###....#.
..#.#..#.#
#...##.#..

Tile 1171:
####...##.
#..##.#..#
##.#..#.#.
.###.####.
..###.####
.##....##.
.#...####.
#.##.####.
####..#...
.....##...

Tile 1427:
###.##.#..
.#..#.##..
.#.##.#..#
#.#.#.##.#
....#...##
...##..##.
...#.#####
.#.####.#.
..#..###.#
..##.#..#.

Tile 1489:
##.#.#....
..##...#..
.##..##...
..#...#...
#####...#.
#..#.#.#.#
...#.#.#..
##.#...##.
..##.##.##
###.##.#..

Tile 2473:
#....####.
#..#.##...
#.##..#...
######.#.#
.#...#.#.#
.#########
.###.#..#.
########.#
##...##.#.
..###.#.#.

Tile 2971:
..#.#....#
#...###...
#.#.###...
##.##..#..
.#####..##
.#..####.#
#..#.#..#.
..####.###
..#.#.###.
...#.#.#.#

Tile 2729:
...#.#.#.#
####.#....
..#.#.....
....#..#.#
.##..##.#.
.#.####...
####.#.#..
##.####...
##..#.##..
#.##...##.

Tile 3079:
#.#.#####.
.#..######
..#.......
######....
####.#..#.
.#...#.##.
#.#####.##
..#.###...
..#.......
..#.###...
)";
char const* pData = R"(Tile 1787:
#..#.#...#
....####..
#.........
...#..#..#
.##....##.
#....#....
...#....##
.......#.#
.......#.#
.##.####.#

Tile 2687:
...#####..
#..##.....
..#.......
##..#..##.
##........
..........
#....#...#
#......#.#
.......#.#
########..

Tile 3359:
##.....#.#
....#.#..#
......##.#
#.##...#..
##...##..#
#....##..#
...#.....#
#.....#...
...#...#.#
##.#..##.#

Tile 1907:
.##..#..##
..#.#.#..#
#.........
..##....#.
...#....#.
#..#.....#
......#..#
##.....#..
#.#.....#.
...#.....#

Tile 1231:
##........
##..#..#.#
.#..#....#
.....##...
....#.#.#.
#.#.#....#
....##.#..
#.#.#....#
....#....#
.###..####

Tile 3301:
.##.#....#
..##.....#
#.........
####...#..
..#.......
#.#....#..
#..##.#.##
#........#
.......#..
.....#....

Tile 3137:
..#...##.#
.#.....#.#
#...#..###
....#..#..
#.........
#.........
#.........
.#..#.#..#
#........#
...#.####.

Tile 3917:
.#....##..
##..#..#..
#........#
#...#.#..#
.#........
.#........
#..#...##.
.#.#...#..
...#...#..
...##....#

Tile 1901:
#.########
..#......#
###.#..#..
.#...##..#
.##.#...#.
.....#...#
#.#..###..
#..#.##.#.
...#.....#
...##.###.

Tile 2399:
####.#.#..
##...#..##
##......##
#.......##
........##
....##..#.
........##
.........#
#.#...####
#.#....#..

Tile 3793:
..##.#.###
.#..#....#
...#......
..#...#...
....##....
..##..#..#
..........
#.........
.#..#....#
##.#.###.#

Tile 1303:
..#..#.##.
...#......
..........
...#...#..
.........#
###......#
.....#..#.
#.........
#...#....#
#.##.#.###

Tile 1997:
###.##..#.
...#.#....
....#....#
##...##...
#...#...#.
##......#.
#.#...##.#
....#....#
#......#.#
##.#.#...#

Tile 3083:
.##.#####.
#.........
........#.
...#.#....
#....#....
#........#
##.....#..
...#..#...
.....#....
###.##..##

Tile 3853:
.#...####.
..........
......#...
#.........
#.#.....#.
#........#
#..#.#....
#..#.....#
.....##..#
####.#####

Tile 2917:
..##.#..##
#.#......#
....##.#..
#..###.#..
.#..#...#.
..#..#..##
..........
#.#..#.#.#
.....#..#.
#####...#.

Tile 1831:
.###.##.##
.#.#.....#
....#....#
#.####....
...##.....
#......#.#
.##.......
..#.....#.
#...##....
..#..###.#

Tile 3371:
..####.###
##....#.#.
......##.#
#.........
........##
.#..#....#
.##......#
#.#....#..
##.....#.#
....#.....

Tile 1433:
.#..#.####
..#..#..#.
...#..#...
#.....#...
#....#...#
.......##.
.........#
#....###..
..#..##.#.
#..#######

Tile 3347:
#....##..#
.#....###.
#.#.#.....
..........
........##
#.......##
.....#...#
#.........
..#......#
..#..###..

Tile 1933:
.##...#.##
....#.....
#..##.#...
......##.#
#..##...##
#........#
#....##...
....#.....
#.......##
#.##.#.#..

Tile 2971:
#.##.#.#.#
.##.......
##.##.#.##
#.........
#.....#..#
.....#..#.
##.......#
#...##.#.#
#........#
#.##....##

Tile 3011:
.#..#.###.
.........#
....#.....
.#.......#
..#......#
........##
#....###..
#......#.#
#.....#..#
.##...##.#

Tile 2069:
..#..#..##
#.........
#....##...
#........#
#.##.....#
.........#
.....##...
......#.##
..#.......
#.##...##.

Tile 1511:
.#.##.##.#
#..##.#...
....#....#
#.#..#....
####.....#
.###..#...
..#...#..#
##..#..##.
#..##...##
##..#.#.##

Tile 1637:
..#..#.###
.......#..
#........#
..#...#...
..........
#........#
#...#..#.#
.##.#.#..#
...#...#.#
#.####.###

Tile 1609:
#..####.##
#....#...#
.....##...
#....#...#
....#...##
#........#
.....#....
....#.....
#.##.#....
#..###.##.

Tile 2749:
##...#.##.
......##..
.........#
##.....#.#
#.##...##.
#.#..#..##
..#...##.#
#...#....#
...#....##
..########

Tile 2141:
....###.#.
....#.##..
.....#..#.
#.#.##...#
........#.
#..#....##
#....#...#
.#.......#
#.....#...
##..#.#..#

Tile 3769:
#..##.##..
##..#..#.#
..##....##
........#.
#..#..#...
##.#..##.#
.#...#...#
###......#
..........
####.###..

Tile 2083:
.###...#..
.#.#......
#...#..#.#
#.........
##.......#
.......#.#
#..#....##
#.......##
#.#.......
.##.#.##.#

Tile 2579:
.###..####
..#..#.#.#
.#........
..........
....##....
#...##...#
##.......#
..........
..........
#..#.##.#.

Tile 3229:
#####..#.#
#.........
#.....#..#
##.....#..
#....#...#
.##...#..#
.....#...#
...##..###
.#.#.....#
...###...#

Tile 2663:
.#.#..####
#......#.#
#.........
##..##.#..
..##...##.
#.#......#
##........
#.#.#...#.
##.....#.#
###...##..

Tile 1709:
.####....#
.#...##...
#.#.#.....
.....#....
..........
..#...#.##
##........
..........
#...##....
#.#.######

Tile 2153:
#####....#
....###..#
.........#
........#.
...#...#..
###..#...#
..##...#..
#.......##
#.....###.
##.###...#

Tile 2803:
#..##..#..
####.#....
#....#.#..
.#.......#
#.#..#..##
#.#...#...
##...##...
.#......##
##.#......
###...####

Tile 1297:
..###.#..#
#...#...#.
....#....#
#...##...#
..#..#...#
#..##.#..#
.#..#.....
#......#..
.#..#..##.
.#.##..###

Tile 3491:
....####..
#...#...##
#...#....#
#...#.#...
##.#.##...
.#........
..##..#.#.
#.....#...
##........
.#.#.#....

Tile 1549:
#..####.#.
#........#
.#........
...#....##
..........
#...##...#
#........#
#...#...##
..#.....#.
#..##..#.#

Tile 3527:
#.###...##
...#...#.#
#..##.#..#
.#.##..#..
........#.
#......###
...#...#..
#.....#.##
#.##......
########.#

Tile 3517:
.#.#..#...
#....#...#
#...#.#...
..#.......
#....#....
#..#.....#
##....#...
#..#.....#
.......#..
#.#..#.##.

Tile 2503:
##....##.#
.....#...#
#.........
#..#.....#
#...#..#..
...#......
#...#...#.
.#.#.....#
#.#.##...#
.#...##.##

Tile 2081:
###......#
#.##.#....
#......##.
.#..#....#
.....#...#
#....#.#.#
.##.......
..........
.......#.#
#.##.#.#.#

Tile 1301:
##.#.#.##.
#.#....#..
.#...#....
#....#..#.
.......#.#
.#.......#
.#.......#
.#.##..#..
.#.....##.
.##.#....#

Tile 3191:
####.#.#..
#.........
.#.....##.
.#...##..#
....#.###.
##.......#
#........#
.#.....#.#
......#..#
#.###.#...

Tile 2539:
.#....#.##
#........#
...##....#
#.........
#....#....
..........
.......#..
....#..#.#
#....##..#
....###.#.

Tile 2819:
#..#####..
....#....#
......#...
###.......
....#....#
#..###...#
#.#..##..#
..#..#...#
###....#..
..#.##.##.

Tile 3989:
###.#..#.#
.....#.##.
.......#.#
.....#.#..
....##..#.
#.......##
#.##...#.#
.....#.###
#.#.#..###
.##..##.#.

Tile 1249:
......#...
#.#......#
....#....#
#.#...####
.........#
........#.
#.........
.......##.
......#..#
.#.##.#.##

Tile 3221:
..##.#.###
#........#
#......#..
##.....#..
....#....#
....#....#
#..#......
..#.....#.
.........#
.#####.##.

Tile 2801:
.##..#.#.#
#.#..#..##
.#....##..
.......#..
#...#.....
#....#....
#.........
.##.#.#.##
.....#...#
.###.#..##

Tile 2137:
#.##.#.##.
#......#..
...##.#..#
.#..#..##.
#.#####.#.
#...#.....
.....#...#
#...###..#
.#.#...#.#
..###.##.#

Tile 3673:
.###.#####
......#..#
.###.#....
..........
.#..#....#
....#.....
.#...#....
#..#....#.
##.#.....#
.#.###.##.

Tile 1747:
####.##.#.
#........#
#.....#..#
.....#...#
....#.#..#
..........
.#.#......
##........
........##
###..###.#

Tile 3121:
#....#..#.
##....#.#.
#..#.....#
.#.......#
..#...#.#.
#....#.##.
.....#...#
#....##...
#.#.......
###.####..

Tile 2521:
#.##..##..
#....#..##
#..#......
#..###....
#.......#.
...#..#..#
#...#....#
....##..##
#.........
.##.#.#.##

Tile 1583:
...#.#####
#..#...#..
##........
.#.......#
......###.
......#..#
#...#....#
.....#.#..
#.........
##.#..###.

Tile 3541:
..##..#..#
..##..##..
#.#..##...
...#.#..##
....#...#.
#.#..#....
.........#
...#.#...#
##..##.#.#
#.#..#..##

Tile 1187:
.......#.#
#.........
.....#..#.
...#..#...
.#.......#
.#.#.#...#
#.#.....#.
#......##.
.#.......#
.##..#####

Tile 3257:
##.#...#.#
#.#.#.....
##....#.##
.....#.#.#
#.....####
#..#...#..
#....#....
.#.......#
.....#..##
####.....#

Tile 2969:
###.##.##.
.....#.#..
##.#....##
#.....#...
...#.#...#
...#..##..
......#..#
..#.......
.#.....#..
..##.#.#.#

Tile 3433:
#.#.#..#..
###.#.#...
.#......##
#.##...#.#
.......#.#
.#.......#
..#......#
.#.....#.#
###.#...##
.##..#.#..

Tile 1427:
.#.#####.#
.#...#.###
..##.###..
..........
..#.......
#....#.#..
.........#
.....##..#
###......#
#.##.#..#.

Tile 1559:
#..#.#.##.
..##.....#
#.##......
#.#.#.####
..........
..##......
#....#....
.###..#.#.
#...#.....
.#.###.##.

Tile 1777:
#....#..#.
#.........
........##
###....#.#
#......#..
....#....#
#.#...#...
##...#....
.........#
##.###.##.

Tile 3169:
.#...#####
.....#....
.##..#.##.
..#.#.#..#
...##....#
#..#......
#...#....#
..#..###.#
.....###..
#..##.###.

Tile 2459:
##.##..#..
.........#
...#..#.#.
#..##..#.#
#.....#..#
..........
#.......#.
..##.....#
..........
.#.......#

Tile 1019:
...#.#.###
#........#
.........#
#.....#..#
.#........
...#.#...#
#.....#..#
.......#..
#.......#.
####.##.#.

Tile 3929:
#.##..###.
#..#....##
..##...#.#
#.#...##.#
#..#...#..
.......#..
.........#
##.#....##
..#....#..
#.###.....

Tile 1447:
####.####.
#..#.#...#
.#....#..#
......#.##
#.........
.#....#.##
....#.....
.......#..
#..#.##..#
#...#.####

Tile 1627:
##.#######
#.#....#..
##.......#
#....#.#..
#.........
#..#...#..
....#.....
.....#....
#.....#.##
##.##.#.#.

Tile 3719:
#.....#...
.#...#..##
..........
###.##.#..
.........#
#.#......#
...#.....#
......##.#
#..#......
#....#.###

Tile 2267:
###..##.##
.....#...#
#.....#...
.....#.#.#
#......###
........##
.......#..
#.#...#...
#.....#.#.
..##...#.#

Tile 1291:
..###...#.
#..#...#..
....#...#.
.##.##.#..
#...#.#.##
.........#
#.....#...
......#.##
.#...#...#
#...#.#...

Tile 3701:
.###.##.#.
......###.
...#......
..#...#.#.
#.##......
.#.#....#.
.###.#..##
.###......
#..##....#
##..#..###

Tile 1607:
...#.##.#.
.....#.#..
...#.##..#
#.........
###......#
.#........
..#.....##
..#.#.#...
.#.##.....
.#.###...#

Tile 3877:
.##.#...##
#...##.#..
#.......##
##.....#..
....#...#.
......#.##
.##..##...
#...#.....
.........#
.##.#..#.#

Tile 1471:
#....##.#.
#...#....#
..#......#
##.###...#
.#.####...
#...#....#
...#....#.
..........
###....#..
.##.#...#.

Tile 2017:
.####.###.
##....#..#
....#.#.##
#...#...##
#.........
#........#
..#.....#.
#........#
..........
...####.#.

Tile 1987:
......#.#.
..#....#..
#.......##
#...#.....
.......#.#
#.#...#...
.#..#.....
.....#.###
..###..#.#
##....#.#.

Tile 1453:
###.###..#
...#......
.....#...#
..#.....##
#........#
.........#
##...#....
.#.......#
..#.#.##..
....#.#.##

Tile 2027:
..#.#..#.#
..#.......
..........
..##.#....
......#...
#...#...##
....#...#.
.....#....
#...#.#.##
#.#####..#

Tile 2411:
##...###..
..#.......
#.#......#
#.#.#...#.
.#.......#
#........#
..#...#..#
#.........
#........#
#..##.#.##

Tile 2939:
....#..#.#
.#.##.#...
.....#..#.
....#...#.
......##.#
...#.##..#
##..##...#
...#.#...#
..#...#..#
...##.####

Tile 1439:
.######.##
..#...#...
.#.#.#..##
#.....#...
.......#.#
#..#.#...#
.....#....
#....#...#
#.##......
...##.....

Tile 3739:
...#...###
....##.#.#
####....##
###.....##
#.........
#.......#.
##........
#.#.......
..##.#....
#####.....

Tile 2957:
###.#.###.
#......#.#
##.......#
...##....#
......#..#
#.#...#...
.....#...#
.......#.#
...#.....#
...#....##

Tile 1861:
##.##....#
.......#..
#........#
...#.....#
#..#.....#
#....#...#
....#....#
#.#.......
....#....#
..###..#..

Tile 3313:
####......
.###.....#
.#.......#
#.......#.
#.##......
###.....##
..#.#.#...
..###....#
###......#
####.#....

Tile 1429:
####....#.
#........#
....#.#...
.......#..
.#...###..
.........#
#........#
#.#......#
#......#.#
##..#..##.

Tile 2707:
.#.#.#....
#........#
.##....#..
....#.#.##
..........
##........
##....#..#
#......#.#
...#..##.#
###..#..##

Tile 2131:
.##.###..#
#...#.....
#.#.##..#.
#.##..#...
#.....####
#..###...#
#####...##
.##.#....#
#....####.
...#######

Tile 3329:
..##.###..
#####....#
..#.#.....
#.........
##...#....
.....#....
#.#...#...
#.########
#...#..#.#
.##.##.#..

Tile 1103:
##.##.###.
#..#...#..
#........#
#........#
....#....#
..#..##..#
..#.#...##
#...##.#..
#.#.##....
.###.##..#

Tile 1201:
#.##.##.##
......#...
..#....#..
..#..###..
...#......
..........
..#...#..#
..#.#.##..
#........#
...#...##.

Tile 2063:
.##..#.##.
.#..##.###
.###...##.
#.#..#..##
#......#.#
#.........
..###.....
........##
.........#
####..##..

Tile 1327:
.#..#.###.
##.......#
.#........
.#.#......
.##.#.#...
.##...#..#
.......#..
#..#.#...#
#.#......#
######..#.

Tile 3779:
...##..##.
#....##..#
..#......#
..#.##....
......##.#
..#.......
.#........
#.#.##....
.##.......
#.###.....

Tile 3061:
....#.###.
#.........
......##.#
#.......##
##....#...
#..#..#...
...#.....#
#.##.....#
#........#
#....#...#

Tile 3323:
#.##.#.#..
...#...#.#
.......#..
#....#....
#......###
........##
#...#.....
#...#.....
#....#...#
####.#...#

Tile 1031:
#..#.#.##.
##...#..##
...#.....#
.....#..##
###.#....#
..#.#....#
..##.#...#
.#...#.#.#
.#........
#.#...#...

Tile 2039:
#.#.###..#
.#.#......
#...##...#
#..#..#...
.#......##
....#....#
#...#.#...
##..##....
#..#.##..#
.#.#...#.#

Tile 1123:
####.###..
#.###..#..
.#...#.#.#
.........#
.....#...#
..##...#.#
...#..#..#
#...##.#.#
..........
#.####.###

Tile 1483:
###.#.####
#.....#.#.
##........
.....#....
...#.....#
......#...
#.........
..#...#..#
#.#.......
...#.#.###

Tile 2053:
..##.#.#..
.....#...#
#..#.....#
##...#...#
.....###..
..........
#...#.....
##.#..#..#
#...##.#.#
..##.#..##

Tile 2879:
...#...###
.....#..#.
#..#......
#.....#...
##...#....
......#...
.#.#......
##...#...#
#..#..#..#
.##...####

Tile 3343:
..#.##...#
#....#..#.
.....#..##
#..#.....#
#...#.....
#.........
#.##....#.
..........
....#.....
.....###..

Tile 3709:
.#..####..
....#.#...
....####..
#...####..
..##..##.#
#....##...
..##......
......#...
#...##.###
.#..#...##

Tile 2531:
..####....
##......#.
.#.......#
#....#....
........##
......#...
##.#.#....
#.........
.##....#..
#..#####.#

Tile 2251:
...#....##
..........
##.......#
#..###...#
#.#..#.###
...##.....
###......#
.##.#.....
.......#.#
####.##.##

Tile 1753:
.#.##...#.
#........#
.#........
#.......#.
...#.#....
......#..#
.#......#.
..........
#..##..#.#
##.##...#.

Tile 2477:
#####.#.##
.......#..
##........
.........#
..........
.........#
##.##....#
#...#.#..#
#......#.#
#.#.#.#...

Tile 1823:
###.#.####
#.#..#..#.
..........
....##....
#.........
#.........
##......##
#........#
..........
##..######

Tile 1801:
###...####
..........
.....##...
.#.#.....#
....#..#..
#.........
#.......#.
#.....#..#
.....#...#
#.##.....#

Tile 2689:
...##....#
..#.......
.#....##.#
...#......
.#.#....##
#.#......#
....####..
..........
....#...##
#.#.#.#...

Tile 1951:
#...##....
#.....#...
.....#...#
.#.#..####
#.#.###..#
#...#..#..
......##..
##........
..#.#....#
.##..#....

Tile 2441:
.##.....#.
......#..#
..#.#.....
#......###
#..#.....#
...#..#.##
..........
......#..#
#..#..##..
###.#.#..#

Tile 1109:
#.#.####..
..#......#
#.....#..#
.#.......#
...#......
..........
#.#....#.#
##.#...#..
#...#....#
##..##.###

Tile 3023:
#..##..#.#
.###.#.###
#....##.##
....#....#
..#.......
#.#.#..#.#
#...#...#.
#...#...##
#......#..
....#.####

Tile 1721:
#...#.##.#
#.........
###..#..#.
..###....#
......##.#
#...#.....
#...#...##
...#.###..
..........
..###.#...

Tile 2887:
..#..#.#.#
.#.....##.
#........#
#.....#..#
.#.....#..
##.....##.
#..#......
.....#...#
#......#..
..###.....

Tile 2659:
#..#.#..#.
#.....#...
....#..#..
#.......#.
.....#..##
........#.
....#....#
#..##.....
..##.#..#.
##.#.##.#.

Tile 2557:
#..#..##..
#.....#...
#....#...#
#...#.....
.#.#......
#......#..
...#...#.#
.......##.
#........#
..#..#.###

Tile 3697:
..##.#...#
#.#.....##
....#....#
.#........
......#...
#......#..
.#.......#
..........
#........#
##.#....##

Tile 3407:
##...###..
#...#.....
#....##...
#........#
#...#.#...
....#..#.#
#...#....#
#........#
#..#.#..##
#..###...#

Tile 2857:
#.###....#
###...###.
.#...#.###
#.#.....##
#.##...##.
#...#..#..
#...###..#
#.#....#..
#..#...#.#
#.#..##.##

Tile 2551:
.#.##.#.#.
#........#
.....#.#..
.#....#...
......#.##
#.......#.
#.#......#
..#.#....#
.#.#...#..
..#.#....#

Tile 1867:
#######...
#.....#.##
#.#...#.#.
#...#.#.##
.....##...
#.#.....##
#.#...#...
....#...#.
#.###....#
#...##..##

Tile 3041:
#.###..#..
..#.......
#...#....#
#....#..##
#......#..
.###..##..
##.##...#.
#..#....##
#........#
##....#.##

Tile 2671:
##..##....
.....#...#
....##....
...#...#..
#...#....#
.......###
.......#.#
.#..#....#
#.....##..
###..##.##

Tile 2543:
..#.#...#.
...#..#...
#.#....##.
...####..#
#.........
##.#....##
##........
#....#....
....#.....
#...##....

Tile 1063:
.#.#.....#
##..#....#
#.##....##
..........
.......#..
#........#
.#......#.
###...#..#
#........#
###.#.#..#

Tile 3539:
.#..###...
..#.....##
.#.##...##
#......#.#
#.#....#..
.#.#.....#
#.##...#..
#.###..###
#.#.#.#.##
.#..######

Tile 2843:
##.....###
#.......##
.........#
.##..##..#
..#.......
#........#
........##
...#.#.#..
.#........
#..##.#.##

Tile 1663:
###.#.#.#.
#.#...#.##
#...#....#
...##.....
.....#...#
#.......#.
#..##.....
...#.#....
..........
..#.#.##..

Tile 1999:
..#.#.#.#.
..........
.......#..
#..#.#....
#.#....#.#
#...#.....
#..##..#..
#...##..##
.#.#....##
.#.....###

Tile 3463:
..##..##.#
#.....#..#
#...##.#..
...##...##
.........#
#.#..#..#.
#....#...#
#........#
#.........
...##...#.

Tile 3823:
#....##..#
..#.##..#.
#..##....#
..........
.........#
......#...
#.#......#
......##.#
..#.......
..##.##.#.

Tile 2777:
.###...###
.........#
##.#...#..
#..#......
#.###.....
.....#..#.
#...#.....
......#...
..##......
...#...##.

Tile 2347:
#...#..#.#
#.#.#.#.##
#....#####
.#...###.#
..##.##.##
.........#
#.........
.....##...
....#.#...
#..#######

Tile 2897:
##.###.##.
.....#..#.
#....##..#
.#..#.#.#.
.#..#....#
#.#.......
##..#....#
......#...
#.#..#....
..#..##...

Tile 2423:
#####.##..
...#......
#.....##.#
.#..##...#
##.###..##
...#...#..
#........#
.........#
#####..###
###.##..#.

Tile 2797:
...#.##.##
#.....#..#
.........#
..###..#.#
....#....#
#.#.......
.#.#....#.
..........
#...#...#.
.#.####..#
)";
