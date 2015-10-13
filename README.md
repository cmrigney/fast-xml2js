# fast-xml2js
In-place replacement for xml2js. This is about 20x faster and makes use of the rapidxml C++ library.

WARNING: This has not been thoroughly tested, so please test yourself before using in production.  It works for our use case.

### Install
Run ```npm install fast-xml2js```

### Using
Simply replace

```var parseString = require('xml2js').parseString;```

with

```var parseString = require('fast-xml2js').parseString;```

