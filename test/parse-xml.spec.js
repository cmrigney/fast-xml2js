'use strict';

const { expect } = require('chai');
const { readFileSync } = require('fs');

const { parseString } = require('../index');

const simpleXml = readFileSync('./test/fixtures/simple.xml', 'utf-8').toString();

describe('parseXml should', () => {

    it('correctly parse xml', done => {
        parseString(simpleXml, (err, data) => {
            expect(err).to.not.exist;
            expect(data).to.eql(
                {
                    data: {
                        description: ['Root description'],
                        entry: [
                            {
                                name: [ 'First entry'],
                                description: ['Entry description']
                            },
                            {
                                name: [ 'Second entry' ],
                                description: [ 'second entry description' ]
                            }
                        ]
                    }
                }
            )

            done();
        })
    })

    it('return error when it got invalid xml', done => {
        parseString('<xcz>dsaf', err => {
            expect(err).to.equal('unexpected end of data');
            done();
        })
    })
})