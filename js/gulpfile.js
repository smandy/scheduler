var bower       = require("bower");
var browserSync = require("browser-sync");
var concat      = require('gulp-concat');
var del         = require("del");
var extreplace  = require("gulp-ext-replace");
var fs          = require('fs');
var gulp        = require("gulp");
var gzip        = require("gulp-gzip");
var minifycss   = require('gulp-minify-css');
var newer       = require('gulp-newer');
var npm         = require('npm');
var open        = require("gulp-open");
var path        = require("path");
var paths       = require('vinyl-paths');
var iceBuilder  = require("gulp-ice-builder");
var uglify      = require("gulp-uglify");
var HttpServer  = require("./HttpServer");
var genDir = 'generated';


gulp.task( 'buildGem', [], function() {
    return gulp.src('../slice/*.ice')
        .pipe(iceBuilder.compile( { dest : genDir} ))
        .pipe( gulp.dest(genDir));
});

gulp.task("watch", [],
    function()
    {
        browserSync();
        HttpServer();
        return gulp.src("./index.html").pipe(open("", {url: "http://127.0.0.1:8080/index.html"}));
    });

