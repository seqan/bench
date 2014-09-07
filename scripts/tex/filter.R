library("reshape")
library("xtable")
library("ggplot2")
library("scales")
#library("Cairo")

INPUT="/Users/esiragusa/Code/seqan/core/apps/ibench/scripts/resources"
OUTPUT="/Users/esiragusa/Documents/Dissertation/plots"

SCALE=0.4
FONT_SIZE=10
POINT_SIZE=2
FONT_FAMILY='Cambria'


### FUNCTIONS ###

options(scipen=999)

checkNA <- function(y)
{
    any(is.na(y))
}

load_file <- function(filename)
{
    if (file.exists(filename))
    {
        # try the main resource file first
        print(paste("read",filename))
        tsvFile = try(read.delim(filename, header=TRUE))
        if (inherits(tsvFile,"try-error"))
        {
            print(paste("EMPTY FILE:", filename))
            return(list(ok=FALSE))
        }
        if (nrow(tsvFile) < 1)
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        if (any(apply(tsvFile, 1, checkNA)))
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        return(list(ok=TRUE,tsv=tsvFile))
    } else
        return(list(ok=FALSE))
}

scientific_10 <- function(x)
{
  parse(text=gsub("e", " %*% 10^", scientific_format()(x)))
}


### PLOT FILTER ###

FILTER_NAMES=c("seeds.exact","seeds.approximate") #,"qgrams.contiguous","qgrams.gapped")
FILTER_LABELS=c("Exact seeds","Approximate seeds") #,"Contiguous q-grams", "Gapped q-grams")

TABLE_FILTERS <- data.frame()
for (FILTER in FILTER_NAMES)
{
  FILENAME_FILTER=paste(paste(INPUT, FILTER, sep='/'), "tsv", sep='.')
  
  if ((R = load_file(FILENAME_FILTER))$ok)
  {
    TABLE_FILTERS <- R$tsv;
  } else
    print(paste("NOT FOUND:", FILENAME_FILTER))
}

