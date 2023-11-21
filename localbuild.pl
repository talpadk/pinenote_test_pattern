$lazyLinking = 1;

my $forceClean = 0;
$argumentHash{"forceclean"} = \$forceClean;

$languageServerTemplateFile = ".ccls_template";

parseArguments();

if ($forceClean){
    $cleanConfirm = 0;
}


$compiler = "gcc";
#$cflags .= " -Wall ";
$codeSuffix = "c";
$includeSuffix = "\\.h|\\.c";
