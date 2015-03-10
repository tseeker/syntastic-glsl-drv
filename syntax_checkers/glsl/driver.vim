if exists("g:loaded_syntastic_glsl_driver_checker")
    finish
endif
let g:loaded_syntastic_glsl_driver_checker = 1

let s:glsl_extensions = {
        \ 'glslf': 'f',
        \ 'glslv': 'v',
        \ 'frag':  'f',
        \ 'vert':  'v',
        \ 'geom':  'g',
        \ 'fp':    'f',
        \ 'vp':    'v'
    \ }

let s:save_cpo = &cpo
set cpo&vim

function! SyntaxCheckers_glsl_driver_GetLocList() dict " {{{1
    let makeprg = self.makeprgBuild({
        \ 'args_before': s:GetShaderType() })

    let errorformat =
        \ "%E%f: 0(%l) : error %m," .
        \ "%W%f: 0(%l) : warning %m," .
        \ "%E%f: (0) : error %m," .
        \ "%W%f: (0) : warning %m"

    return SyntasticMake({
        \ 'makeprg': makeprg,
        \ 'errorformat': errorformat })
endfunction " }}}1

function! s:GetShaderType() " {{{2
    let save_view = winsaveview()
    let old_foldenable = &foldenable
    let old_lazyredraw = &lazyredraw

    let &lazyredraw = 1
    let &foldenable = 0
    call cursor(1, 1)

    let magic = '\m\C^// type:\s*'
    let line = search(magic, 'c')

    call winrestview(save_view)
    let &foldenable = old_foldenable
    let &lazyredraw = old_lazyredraw

    if line
        let profile = matchstr(getline(line), magic . '\zs.*')
    else
        let profile = get(s:extensions, tolower(expand('%:e', 1)), 'v')
    endif

    return profile
endfunction " }}}2

call g:SyntasticRegistry.CreateAndRegisterChecker({
    \'filetype': 'glsl',
    \'name': 'driver',
    \'exec': 'check-glsl'})

let s:ownpath = expand( '<sfile>:p:h:h:h' )
let g:syntastic_glsl_driver_exec = s:ownpath . '/check-glsl'

let &cpo = s:save_cpo
unlet s:save_cpo

" vim: set sw=4 sts=4 et fdm=marker:

