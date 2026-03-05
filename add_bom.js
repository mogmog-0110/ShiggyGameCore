const fs = require('fs');
const path = require('path');

const files = process.argv.slice(2);
for (const file of files) {
    const buf = fs.readFileSync(file);
    if (buf[0] !== 0xEF || buf[1] !== 0xBB || buf[2] !== 0xBF) {
        fs.writeFileSync(file, Buffer.concat([Buffer.from([0xEF, 0xBB, 0xBF]), buf]));
        console.log('BOM added:', file);
    } else {
        console.log('BOM exists:', file);
    }
}
