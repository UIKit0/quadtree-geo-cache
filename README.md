# Quadtree Geo-Cache
This cache is represented as a quadtree, and stores data in buckets, indexed by lat-lng pairs. 
The buckets have an upper bound, and will break up into four regions when the limit is reached.
This way, range queries are bounded, quick, and likely to return the closest bucket without too much computation.

# Quadtree Protocol
## Quad Tree Commands

### QTCREATE
**Arguments:** `lat lng lat-delta lng-delta`  
**Response:** `qt-id:ERRNO`

### QTLIST
**Arguments:** `None`  
**Response:** `number-returned qt-idN latN lngN lat-deltaN lng-deltaN`

### QTDESTROY
**Arguments:** `qt-id`  
**Response:** `0:ERRNO`

### QTDESTROYALL
**Arguments:** `0:nonce`  
**Response:** `nonce:0:ERRNO`
**Note:** Two requests are required to eveict the entire cache. 
The first sends a nonce of 0 and receices a nonce from the server. 
The second sends the nonce to the server and receives an OK or FAIL.  

## Record Insert Commands

### RCREATE
**Arguments:** `lat lng data-size data`  
**Response:** `qt-id:ERRNO`  
**Note:** This will automatically determine the best quadtree to insert the record into.
If you're looking for more control, use RCREATEQT. 
This will also create a new record, even if one exists with the same lat and lng. 
To avoid duplicates, use RCREATENX. (unique)  

### RCREATEQT
**Arguments:** `qt-id lat lng data-size data`  
**Response:** `0:ERRNO`  
**Note:** Like RCREATE, this will create a new record even if one exists with the same lat and lng.
To avoid duplicates, use RCREATENXQT. (unique)  

### RCREATENX
**Arguments:** `lat lng data-size data`  
**Response:** `qt-id:0:ERRNO`  
**Note:** This will automatically determine the best quadtree to insert the record into.
If you're looking for more control, use RCREATENXQT.
This will return 0 if a duplicate exists.  

### RCREATENXQT
**Arguments:** `qt-id lat lng data-size data`  
**Response:** `0:ERRNO`  
**Note:** This will insert the record into the specified quadtree, and will return 0 if it is a duplicate.  

### RAPPEND
**Arguments:** `lat lng data-size data`  
**Response:** `qt-id:0:ERRNO`  
**Note:** This will look for a matching record and append the data to it.
If a match is not found, a new record will be created and the data will be appened.
If multiple records with the same lat-lng pair exist, the data will be appened to the first encountered.  

### RAPPENDQT
**Arguments:** `qt-id lat lng data-size data`  
**Response:** `0:ERRNO`  
**Note:** The same as RAPPEND, however this will look in the specified quadtree.  

## Record Query Commands

### RQUERY
**Arguments:** `lat lng`  
**Response:** `number-returned rlatN rlngN data-sizeN dataN`  
**Note:** This will find the bucket closest to the given lat-lng pair, and return all of the records in it.

