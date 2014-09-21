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

FILTER_NAMES=c("seeds_0","seeds_1","seeds_2","qgrams_0","qgrams_1","qgrams_2","qgrams_gapped")
FILTER_LABELS=c("Exact seeds",
                "1-Apx seeds",
                "2-Apx seeds",
                "Contig q-grams (t >= 1)",
                "Contig q-grams (t >= 2)",
                "Contig q-grams (t >= 4)",
                "Gapped q-grams")

DATASET='celegans'
ALPHABET='dna'
ALPHSIZE=4
PLENGTH=100
DISTANCES=c('hamming','edit')

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


### PLOT PPV ###

FILENAME_OCCS=paste(paste(INPUT, 'filter_occurrences', sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_OCCS))$ok)
{
  TABLE_OCCS <- R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_OCCS))

#table_occs = subset(TABLE_OCCS, alphabet==ALPHABET & dataset==DATASET & plength==PLENGTH, select=c(filter, errors, distance, verifications, matches))
#TABLE_FILTERS = merge(TABLE_FILTERS, TABLE_OCCS, by=c('alphabet', 'dataset', 'pcount', 'plength', 'errors', 'distance'), all=TRUE)

for (DISTANCE in DISTANCES)
{
  PLOT_PPV=paste(paste(OUTPUT, "ppv", sep='/'), ALPHABET, DATASET, DISTANCE, PLENGTH, "pdf", sep='.')
  
  table_ppv = subset(TABLE_OCCS, alphabet==ALPHABET & dataset==DATASET & plength==PLENGTH & distance==DISTANCE, select=c(filter, errors, verifications, duplicates, occurrences))
  table_ppv$ppv <- table_ppv$occurrences / table_ppv$verification
  
  ggplot() +
    geom_line(data=table_ppv, aes(x=errors, y=ppv, group=filter, shape=filter, color=filter), linetype='solid') +
    geom_point(data=table_ppv, aes(x=errors, y=ppv, group=filter, shape=filter, color=filter), size=POINT_SIZE) +
    scale_shape_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
    scale_color_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
    xlab("Errors") +
    ylab("PPV (matches/verifications)") +
    theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)
  
  ggsave(file=PLOT_PPV, scale=SCALE, device=cairo_pdf) 
}


### PLOT RUNTIME ###

FILENAME_FILTER=paste(paste(INPUT, 'filter_only', sep='/'), "tsv", sep='.')
FILENAME_VERIFY=paste(paste(INPUT, 'filter_verify', sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_FILTER))$ok)
{
  TABLE_FILTER <- R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_FILTER))

if ((R = load_file(FILENAME_VERIFY))$ok)
{
  TABLE_VERIFY <- R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VERIFY))

TABLE_RUNTIME = merge(TABLE_FILTER, TABLE_VERIFY, by=c('alphabet', 'dataset', 'pcount', 'plength', 'errors', 'distance', 'filter'), all=TRUE)
TABLE_RUNTIME = rename(TABLE_RUNTIME, c("time.x"="ftime", "time.y"="time"))

for (DISTANCE in DISTANCES)
{
  table_runtime = subset(TABLE_RUNTIME, alphabet==ALPHABET & dataset==DATASET & distance==DISTANCE & plength==PLENGTH, select=c(filter, errors, pcount, time, ftime))
  table_runtime <- transform(table_runtime, time = time / pcount, ftime = ftime / pcount)
  
  PLOT_RUNTIME=paste(paste(OUTPUT, "filter_runtime", sep='/'), ALPHABET, DATASET, DISTANCE, PLENGTH, "pdf", sep='.')
  
  ggplot() +
    geom_line(data=table_runtime, aes(x=errors, y=time, group=filter, shape=filter, color=filter), linetype='solid') +
    geom_point(data=table_runtime, aes(x=errors, y=time, group=filter, shape=filter, color=filter), size=POINT_SIZE) +
    geom_line(data=table_runtime, aes(x=errors, y=ftime, group=filter, shape=filter, color=filter), linetype='dashed') +
    geom_point(data=table_runtime, aes(x=errors, y=ftime, group=filter, shape=filter, color=filter), size=POINT_SIZE-1) +
    scale_shape_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
    scale_color_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
    xlab("Errors") +
    ylab("Time (seconds/pattern)") +
    scale_y_log10(labels=scientific_10) +
    theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)
  
  ggsave(file=PLOT_RUNTIME, scale=SCALE, device=cairo_pdf) 
}


### PLOT ONLINE VS OFFLINE RUNTIME ###

# ERRORS=4
# 
# FILENAME_ONOFF=paste(paste(INPUT, 'onoff', sep='/'), "tsv", sep='.')
# PLOT_ONOFF=paste(paste(OUTPUT, "onoff", sep='/'), ALPHABET, DATASET, DISTANCE, PLENGTH, "pdf", sep='.')
# 
# if ((R = load_file(FILENAME_ONOFF))$ok)
# {
#   TABLE_ONOFF <- R$tsv;
# } else
#   print(paste("NOT FOUND:", FILENAME_ONOFF))
# 
# table_onoff = subset(TABLE_ONOFF, alphabet==ALPHABET & dataset==DATASET & plength==PLENGTH & errors==ERRORS, select=c(filter, pcount, time))
# table_onoff <- transform(table_onoff, time = time / pcount)
# 
# ggplot() +
#   geom_line(data=table_onoff, aes(x=pcount, y=time, group=filter, shape=filter, color=filter), linetype='solid') +
#   geom_point(data=table_onoff, aes(x=pcount, y=time, group=filter, shape=filter, color=filter), size=POINT_SIZE) +
#   scale_shape_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
#   scale_color_discrete(name="Filter", breaks=FILTER_NAMES, labels=FILTER_LABELS) +
#   xlab("Patterns") +
#   ylab("Time (seconds/patterns)") +
#   scale_x_log10() +
#   theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)
# 
# ggsave(file=PLOT_ONOFF, scale=SCALE, device=cairo_pdf) 


### PLOT VERIFICATIONS ###

# TABLE_VERIFICATIONS <- data.frame()
# for (FILTER in FILTER_NAMES)
# {
#   FILENAME_FILTER=paste(paste(INPUT, FILTER, sep='/'), "tsv", sep='.')
#   
#   if ((R = load_file(FILENAME_FILTER))$ok)
#   {
#     TABLE_VERIFICATIONS <- R$tsv;
#   } else
#     print(paste("NOT FOUND:", FILENAME_FILTER))
# }

