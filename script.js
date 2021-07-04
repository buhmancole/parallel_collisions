let canvas = null;
let context = null;
let prevTime = null;
let sats = null;

let file = null;
let reader = null;

const delay = ms => new Promise(res => setTimeout(res, ms));

function initialize(input) {
    let file = input.files[0];
    let reader = new FileReader();
    reader.readAsText(file);
    reader.onload = function() {
        sats = JSON.parse(reader.result);
        sats = sats.sats;
        run();
    }
}

function newXY(x, y, r) {
    let dist = Math.sqrt(x*x + y*y) + r;
    let rad = Math.atan(y/x);
    x = (x < 0 ? -dist*Math.cos(rad) : dist*Math.cos(rad));
    y = dist*Math.sin(rad);
    return [x+500, y+500];
}

const run = async() => {
    canvas = document.getElementById('canvas');
    context = canvas.getContext('2d');
    
    prevTime = performance.now();
	
    // draw the sats
    for (let i = 0; i < sats.length; i++) {
        await delay(200);
        let cycle = sats[i];

        // clear canvas and draw earth
        context.clearRect(0, 0, canvas.width, canvas.height);
        context.stokeStyle = 'rgb(0,0,0)';
        context.fillStyle = 'rgb(0,0,0)';
        context.fillRect(0,0,canvas.width,canvas.height);
        context.strokeRect(0,0,canvas.width,canvas.height);

        context.beginPath();
        context.arc(500, 500, 60, 0, 2*Math.PI);
        context.closePath();
        context.strokeStyle = 'rgb(0, 255, 0)';
        context.fillStyle = 'rgb(0, 255, 0)';
        context.lineWidth = 1;
        context.stroke();
        context.fill();
        // draw satellites
        context.beginPath();
        for (let j = 0; j < cycle.length; ++j) {
            let sat = cycle[j];
            if (sat[3] == 0) {
                let coords = newXY(sat[0], sat[1], 60);
                let r = sat[2]/5;
                context.moveTo(coords[0]+r, coords[1]);
                context.arc(coords[0], coords[1], r, 0, 2*Math.PI);
            }
        }
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.closePath();
        context.stroke();

        // draw collisions
        context.beginPath();
        for (let j = 0; j < cycle.length; ++j) {
            let sat = cycle[j];
            if (sat[3] != 0) {
                let coords = newXY(sat[0], sat[1], 60);
                let r = sat[2];
                context.moveTo(coords[0]+r, coords[1]);
                context.arc(coords[0], coords[1], r, 0, 2*Math.PI);
            }
        }
        context.closePath();
        context.strokeStyle = 'rgb(255, 0, 0)';
        context.fillStyle = 'rgb(255,0,0)';
        context.stroke();
        context.fill();
    }
    //context.strokeStyle = 'rgb(255, 255, 255)';
}
