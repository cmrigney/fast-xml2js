var fastxml2js = require('./fast-xml2js/fast-xml2js.node');

/**
 * Callback after xml is parsed
 * @callback parseCallback
 * @param {string} err Error string describing an error that occurred
 * @param {object} obj Resulting parsed JS object
 */

/**
 * Parses an XML string into a JS object
 * @param  {string} xml
 * @param  {parseCallback} cb
 */
function parseString(xml, cb) {
    return fastxml2js.parseString(xml, function(err, data) {
        //So that it's asynchronous
        process.nextTick(function() {
            cb(err, data);
        });
    });
};

exports.parseString = parseString;

