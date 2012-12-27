# Quadtree Geo-Cache
This cache is represented as a quadtree, and stores data in buckets, indexed by lat-lng pairs. 
The buckets have an upper bound, and will break up into four regions when the limit is reached.
This way, range queries are bounded, quick, and likely to return the closest bucket without too much computation.
